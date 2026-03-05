#include "WebUI.h"
#include "Config.h"
#include "Logger.h"
#include "WiFiManager.h"
#include "TelegramManager.h"
#include "PowerMonitor.h"
#include "OtaManager.h"
#include "TimeUtils.h"

// ============================================================
//  CSS
// ============================================================
static const char CSS[] PROGMEM = R"css(
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#0d1117;color:#e6edf3;font-family:'Segoe UI',system-ui,sans-serif;font-size:14px;min-height:100vh}
a{color:#58a6ff;text-decoration:none}
/* Header — центрований */
.hdr{background:#161b22;border-bottom:1px solid #30363d;padding:10px 20px;
     display:flex;justify-content:center;align-items:center;gap:12px;
     position:sticky;top:0;z-index:200;flex-wrap:wrap}
.hdr-title{font-size:17px;font-weight:700;color:#58a6ff}
.badge{padding:3px 10px;border-radius:20px;font-size:11px;font-weight:600;letter-spacing:.3px}
.b-on{background:#1a4731;color:#3fb950}.b-off{background:#3d1f1f;color:#f85149}
.b-warn{background:#3d2f1f;color:#d29922}.b-info{background:#1f3a6e;color:#58a6ff}
.b-batt{background:#2d2d0f;color:#e3c84a}
/* Nav — центрований */
.nav{background:#161b22;border-bottom:1px solid #30363d;padding:4px 16px;
     display:flex;justify-content:center;flex-wrap:wrap;gap:2px}
.nav a{color:#8b949e;padding:7px 14px;border-radius:6px;font-size:13px;transition:.15s}
.nav a:hover,.nav a.on{background:#21262d;color:#e6edf3}
.wrap{max-width:940px;margin:0 auto;padding:16px}
/* Cards */
.card{background:#161b22;border:1px solid #30363d;border-radius:10px;padding:18px;margin-bottom:14px}
.card-title{font-size:11px;color:#8b949e;text-transform:uppercase;letter-spacing:.7px;margin-bottom:14px;
            padding-bottom:8px;border-bottom:1px solid #21262d;font-weight:600}
/* Grids */
.grid {display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:10px}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.grid3{display:grid;grid-template-columns:1fr 1fr 1fr;gap:12px}
/* Stat tiles */
.stat{background:#0d1117;border:1px solid #21262d;border-radius:8px;padding:12px}
.sl{font-size:11px;color:#6e7681;text-transform:uppercase;letter-spacing:.4px;margin-bottom:5px}
.sv{font-size:18px;font-weight:700}
.sv.g{color:#3fb950}.sv.r{color:#f85149}.sv.b{color:#58a6ff}
.sv.y{color:#d29922}.sv.s{font-size:13px;color:#e6edf3}.sv.gold{color:#e3c84a}
/* Battery bar */
.batt-wrap{margin-top:8px}
.batt-bar{background:#21262d;border-radius:6px;height:10px;overflow:hidden}
.batt-fill{height:100%;border-radius:6px;transition:.6s}
.batt-hi{background:#3fb950}.batt-mid{background:#d29922}.batt-lo{background:#f85149}
/* Forms */
.fg{margin-bottom:12px}
.fg label{display:block;margin-bottom:4px;color:#8b949e;font-size:11px;text-transform:uppercase;letter-spacing:.4px;text-align:left}
.fg input,.fg select,.fg textarea{width:100%;box-sizing:border-box}
.hint{display:block;margin-top:4px;color:#6e7681;font-size:11px;line-height:1.4}
input[type=text],input[type=password],input[type=number],input[type=url],select,textarea{
  width:100%;background:#0d1117;border:1px solid #30363d;color:#e6edf3;
  padding:8px 11px;border-radius:6px;font-size:14px;outline:none;transition:.15s;font-family:inherit}
input:focus,select:focus,textarea:focus{border-color:#58a6ff;box-shadow:0 0 0 2px rgba(88,166,255,.12)}
select option{background:#0d1117}
/* Toggle */
.tgl{display:flex;flex-direction:row;align-items:center;gap:10px;cursor:pointer;user-select:none;padding:6px 0;width:fit-content}
.tgl input{display:none}
.tgl-sl{width:38px;height:21px;background:#21262d;border-radius:11px;position:relative;transition:.2s;flex-shrink:0;align-self:center}
.tgl-sl::after{content:'';position:absolute;top:3px;left:3px;width:15px;height:15px;background:#6e7681;border-radius:50%;transition:.2s}
.tgl input:checked+.tgl-sl{background:#1a4731}
.tgl input:checked+.tgl-sl::after{background:#3fb950;left:20px}
.tgl-txt{font-size:13px;color:#e6edf3;line-height:1.3;align-self:center}
/* Buttons */
.btn{display:inline-flex;align-items:center;justify-content:center;gap:6px;
     padding:8px 16px;border-radius:6px;border:none;cursor:pointer;
     font-size:13px;font-weight:500;text-decoration:none;transition:.15s;
     white-space:nowrap;line-height:1;min-height:34px}
.btn:disabled{opacity:.5;cursor:not-allowed}
.btn-p{background:#1f6feb;color:#fff}.btn-p:hover:not(:disabled){background:#388bfd}
.btn-s{background:#21262d;color:#e6edf3;border:1px solid #30363d}.btn-s:hover:not(:disabled){background:#30363d}
.btn-g{background:#1a4731;color:#3fb950;border:1px solid #2ea043}.btn-g:hover:not(:disabled){background:#1f5937}
.btn-d{background:#3d1f1f;color:#f85149;border:1px solid #da3633}.btn-d:hover:not(:disabled){background:#4a1f1f}
.btn-y{background:#3d2f1f;color:#d29922;border:1px solid #9e6a03}.btn-y:hover:not(:disabled){background:#4a3a1f}
.btn-sm{padding:6px 12px;font-size:12px;min-height:30px}
/* Alerts */
.alert{padding:10px 14px;border-radius:6px;margin-bottom:12px;font-size:13px;line-height:1.5}
.a-ok  {background:#1a4731;border:1px solid #2ea043;color:#3fb950}
.a-err {background:#3d1f1f;border:1px solid #da3633;color:#f85149}
.a-info{background:#1f3a6e;border:1px solid #1f6feb;color:#58a6ff}
.a-warn{background:#3d2f1f;border:1px solid #9e6a03;color:#d29922}
/* Log */
.log-row{padding:7px 10px;border-left:3px solid #30363d;margin-bottom:4px;
         background:#0d1117;border-radius:0 6px 6px 0;font-size:12px;line-height:1.5}
.log-ts{color:#6e7681;font-size:11px;margin-right:6px}
/* Queue */
.q-item{padding:8px 12px;background:#0d1117;border:1px solid #21262d;border-radius:6px;margin-bottom:6px;font-size:12px}
.q-item.ok  {border-left:3px solid #2ea043}
.q-item.wait{border-left:3px solid #9e6a03}
/* OTA drop */
.drop{border:2px dashed #30363d;border-radius:10px;padding:36px 20px;text-align:center;cursor:pointer;transition:.2s}
.drop:hover,.drop.drag{border-color:#58a6ff;background:#0d1117}
.prog{background:#0d1117;border-radius:8px;height:7px;overflow:hidden;margin-top:8px}
.prog-b{height:100%;background:#1f6feb;border-radius:8px;transition:.4s}
/* Tabs */
.tabs{display:flex;gap:4px;margin-bottom:14px}
.tab{padding:6px 14px;border-radius:6px;cursor:pointer;font-size:13px;
     background:#0d1117;border:1px solid #30363d;color:#8b949e;transition:.15s}
.tab.on{background:#1f3a6e;border-color:#1f6feb;color:#58a6ff}
.tab-content{display:none}.tab-content.on{display:block}
/* WiFi scan list */
.wnet{display:flex;align-items:center;gap:10px;padding:8px 10px;background:#0d1117;
      border:1px solid #21262d;border-radius:6px;margin-bottom:5px;cursor:pointer;transition:.15s}
.wnet:hover{border-color:#30363d;background:#161b22}
.wnet.saved{border-color:#2ea043}
.rssi-bar{width:40px;height:6px;background:#21262d;border-radius:3px;flex-shrink:0}
.rssi-fill{height:100%;border-radius:3px}
.wnet-ssid{flex:1;font-size:13px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.wnet-info{font-size:11px;color:#6e7681;flex-shrink:0}
.lock{font-size:11px;color:#6e7681}
/* Save bar */
.save-bar{display:flex;gap:8px;align-items:center;justify-content:center;padding:14px 0 20px;flex-wrap:wrap}
/* Misc */
.row{display:flex;gap:8px;align-items:flex-end}.row>*{flex:1}
.divider{border:none;border-top:1px solid #21262d;margin:14px 0}
table{width:100%;border-collapse:collapse}
th,td{padding:7px 12px;text-align:left;border-bottom:1px solid #21262d;font-size:13px}
th{color:#6e7681;font-size:11px;text-transform:uppercase;font-weight:500}
.actions{display:flex;gap:8px;flex-wrap:wrap;justify-content:center}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}
.pulse{animation:pulse 1.4s infinite}
@keyframes spin{to{transform:rotate(360deg)}}
.spin{display:inline-block;animation:spin .8s linear infinite}
@media(max-width:640px){.grid2,.grid3{grid-template-columns:1fr}.hdr{gap:8px}}
</style>
)css";

// ============================================================
//  JS live update
// ============================================================
static const char JS_LIVE[] PROGMEM = R"js(
<script>
const $=id=>document.getElementById(id);
const set=(id,v,cls)=>{const e=$(id);if(!e)return;e.textContent=v;if(cls!==undefined)e.className=cls};
function battClass(p){return p>=50?'batt-hi':p>=20?'batt-mid':'batt-lo'}
async function refreshStatus(){
  try{
    const d=await(await fetch('/api/status')).json();
    set('s-power',d.power?'✅ ПРИСУТНЄ':'❌ ВІДСУТНЄ','sv '+(d.power?'g':'r'));
    set('s-wifi', d.wifi?'📶 '+d.ssid:'❌ Відключено');
    set('s-ip',   d.ip);
    set('s-time', d.time);
    set('s-up',   d.uptime);
    set('s-ntp',  d.ntp?'✅ Синхр.':'⚠️ Ні','sv '+(d.ntp?'g':'y'));
    set('s-q',    d.queue,'sv b');
    set('s-sent', d.total);
    set('s-del',  d.delivered,'sv g');
    set('s-rssi', d.rssi?d.rssi+' dBm':'—');
    set('s-err',  d.error||'Немає','sv s');
    set('s-off',  d.off_dur||'—', d.off_dur?'sv r pulse':'sv s');
    // Батарея
    if(d.batt_en){
      const bt=$('batt-tile');
      if(bt){bt.style.display='';bt.style.removeProperty('display');}
      set('s-battv', d.batt_v+' В','sv gold');
      set('s-battp', d.batt_p+'%','sv '+(d.batt_p>=50?'g':d.batt_p>=20?'y':'r')+' ');
      const bf=$('batt-fill-d');
      if(bf){bf.style.width=d.batt_p+'%';bf.className='batt-fill '+battClass(d.batt_p)}
    } else {
      const bt=$('batt-tile');if(bt)bt.style.display='none';
    }
  }catch(e){}
}
document.addEventListener('DOMContentLoaded',()=>{refreshStatus();setInterval(refreshStatus,3000)});
</script>
)js";

// ============================================================
//  Helpers
// ============================================================
String WebUI::_esc(const String& s){
    String r;r.reserve(s.length()+8);
    for(char c:s){
        if(c=='<')r+="&lt;";else if(c=='>')r+="&gt;";
        else if(c=='&')r+="&amp;";else if(c=='"')r+="&quot;";
        else if(c=='\'')r+="&#39;";  // ВАЖЛИВО: екранувати для value='...'
        else r+=c;
    }
    return r;
}
String WebUI::_checked(bool v){return v?" checked":"";}
String WebUI::_sel(uint8_t cur,uint8_t val,const char* lbl){
    return "<option value='"+String(val)+"'"+(cur==val?" selected":"")+">"+lbl+"</option>";
}
void WebUI::_redirect(const String& url,int code){
    _srv.sendHeader("Location",url);_srv.send(code,"text/plain","");
}

// ============================================================
//  RSSI → 0-100%
// ============================================================
static int rssiPct(int32_t rssi){
    if(rssi<=-100)return 0;
    if(rssi>=-50) return 100;
    return 2*(rssi+100);
}
static const char* rssiColor(int pct){
    if(pct>=60)return "#3fb950";
    if(pct>=30)return "#d29922";
    return "#f85149";
}

// ============================================================
//  Page template
// ============================================================
String WebUI::_head(const String& title) const {
    String h;h.reserve(1400);
    h+=F("<!DOCTYPE html><html lang='uk'><head>"
         "<meta charset='UTF-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<title>");
    h+=_esc(title);
    h+=F(" — Power Monitor</title>");
    h+=FPSTR(CSS);
    h+=F("</head><body>");
    // Header — центрований
    h+=F("<div class='hdr'>"
         "<span style='font-size:22px'>⚡</span>"
         "<div class='hdr-title'>Power Monitor</div>");
    h+="<span class='badge b-info' style='font-size:10px'>" BOARD_NAME "</span>";
    h+="<span class='badge "+(PM().powerPresent()?String("b-on"):String("b-off"))+"'>";
    h+=PM().powerPresent()?"ON":"OFF";
    h+="</span>";
    if(PM().battEnabled()){
        int bp=PM().battPercent();
        h+="<span class='badge b-batt'>🔋 "+String(PM().battVoltage(),1)+"В "+String(bp)+"%</span>";
    }
    if(!WM().isConnected())h+="<span class='badge b-warn'>WiFi ⚠️</span>";
    h+="<span style='color:#6e7681;font-size:11px'>v" FW_VERSION "</span>";
    h+="</div>";
    return h;
}

String WebUI::_nav(const String& active) const {
    struct P{const char* url;const char* lbl;};
    const P pages[]={{"/","📊 Дашборд"},{"/config","⚙️ Налаштування"},{"/logs","📋 Журнал"},{"/ota","🔄 OTA"}};
    String n="<div class='nav'>";
    for(auto& p:pages)
        n+="<a href='"+String(p.url)+"'"+(active==p.url?" class='on'":"")+">"+p.lbl+"</a>";
    n+="</div>";
    return n;
}
String WebUI::_foot(){return String(FPSTR(JS_LIVE))+"</body></html>";}

// ============================================================
//  /api/status
// ============================================================
void WebUI::_handleStatus(){
    String j;j.reserve(800);

    // --- Живлення ---
    bool   pwr    = PM().powerPresent();
    uint32_t offMs = PM().offDurationMs();
    uint32_t onMs  = PM().onDurationMs();

    // RSSI → відсотки (−50дБм=100%, −100дБм=0%)
    int rssi    = WM().isConnected() ? WM().rssi() : -100;
    int rssiPct = WM().isConnected() ? constrain((rssi + 100) * 2, 0, 100) : 0;

    j+='{';
    // Базові поля (сумісні зі старим форматом)
    j+="\"power\":"      +String(pwr?"true":"false")+',';
    j+="\"state\":\""   +String(pwr?"on":"off")+"\",";       // HA binary_sensor
    j+="\"wifi\":"       +String(WM().isConnected()?"true":"false")+',';
    j+="\"ssid\":\""    +_esc(WM().ssid())+"\",";
    j+="\"ip\":\""      +WM().ip()+"\",";
    j+="\"rssi\":"      +String(rssi)+',';
    j+="\"rssi_pct\":"  +String(rssiPct)+',';                // 0-100%
    j+="\"ntp\":"       +String(Time().isSynced()?"true":"false")+',';
    j+="\"time\":\""   +Time().now()+"\",";
    j+="\"uptime\":\""  +Time().duration(millis())+"\",";
    j+="\"uptime_sec\":" +String(millis()/1000UL)+',';       // числове для графіків

    // Telegram
    j+="\"queue\":"     +String(Tg().queueCount())+',';
    j+="\"total\":"     +String(Tg().totalSent())+',';
    j+="\"delivered\":" +String(Tg().totalDelivered())+',';

    // Тривалість відключення (рядок + секунди)
    j+="\"off_dur\":\"" +((!pwr && offMs>0) ? Time().duration(offMs) : String(""))+"\",";
    j+="\"off_dur_sec\":" +String(!pwr && offMs>0 ? offMs/1000UL : 0UL)+',';
    j+="\"on_dur_sec\":"  +String(pwr  && onMs>0  ?  onMs/1000UL : 0UL)+',';

    // Статистика відключень
    j+="\"outage_count\":" +String(PM().outageCount())+',';
    j+="\"last_off_ts\":"  +String((uint32_t)PM().lastOffTimestamp())+',';  // unix ts
    j+="\"last_on_ts\":"   +String((uint32_t)PM().lastOnTimestamp())+',';

    // Батарея
    bool   battEn = PM().battEnabled();
    float  battV  = PM().battVoltage();
    int    battP  = PM().battPercent();
    bool   charging = pwr && battEn;   // живлення є → батарея (ймовірно) заряджається
    j+="\"batt_en\":"      +String(battEn?"true":"false")+',';
    j+="\"batt_v\":"       +String(battV,2)+',';             // число, не рядок
    j+="\"batt_v_str\":\"" +String(battV,2)+"\",";           // рядок для сумісності
    j+="\"batt_p\":"       +String(battP)+',';
    j+="\"batt_low\":"     +String(PM().battLow()?"true":"false")+',';
    j+="\"batt_charging\":" +String(charging?"true":"false")+',';

    j+="\"error\":\"\"";
    j+='}';
    _srv.sendHeader("Cache-Control","no-cache, no-store");
    _srv.send(200,"application/json",j);
}

// ============================================================
//  /api/scan  — повертає JSON список мереж
// ============================================================
void WebUI::_handleScan(){
    // Запускаємо синхронний скан (~2-3с)
    WM().scanNetworks();
    String j="[";
    for(int i=0;i<WM().scannedCount();i++){
        const ScannedNet& n=WM().scannedNet(i);
        if(i>0)j+=',';
        j+="{\"ssid\":\""+_esc(n.ssid)+"\",";
        j+="\"rssi\":"+String(n.rssi)+",";
        j+="\"enc\":"+String(n.encType==0?"false":"true")+",";
        j+="\"saved\":"+String(n.saved?"true":"false")+"}";
    }
    j+="]";
    _srv.sendHeader("Cache-Control","no-cache, no-store");
    _srv.send(200,"application/json",j);
}

// ============================================================
//  Dashboard
// ============================================================
void WebUI::_handleRoot(){
    String h=_head("Дашборд")+_nav("/");
    h+="<div class='wrap'>";
    h+="<div class='card'><div class='card-title'>📊 Поточний стан</div><div class='grid'>";

    auto stat=[&](const char* id,const char* lbl,const String& val,const char* cls){
        h+="<div class='stat'><div class='sl'>"+String(lbl)+"</div>";
        h+="<div id='"+String(id)+"' class='sv "+String(cls)+"'>"+val+"</div></div>";
    };
    stat("s-power","Живлення",PM().powerPresent()?"✅ ПРИСУТНЄ":"❌ ВІДСУТНЄ",PM().powerPresent()?"g":"r");
    stat("s-wifi","WiFi",WM().ssid(),"b");
    stat("s-ip","IP адреса",WM().ip(),"s");
    stat("s-time","Час",Time().now(),"s");
    stat("s-up","Uptime",Time().duration(millis()),"s");
    stat("s-ntp","NTP",Time().isSynced()?"✅ Синхр.":"⚠️ Ні",Time().isSynced()?"g":"y");
    stat("s-q","Черга TG",String(Tg().queueCount()),"b");
    stat("s-sent","Відправлено",String(Tg().totalSent()),"s");
    stat("s-del","Доставлено",String(Tg().totalDelivered()),"g");
    stat("s-rssi","WiFi сигнал",WM().isConnected()?String(WM().rssi())+" dBm":"—","s");
    uint32_t initOffMs = PM().offDurationMs();
    stat("s-off","Відсутнє",
        (PM().powerPresent()||initOffMs==0) ? "—" : Time().duration(initOffMs),
        (PM().powerPresent()||initOffMs==0) ? "s" : "r pulse");
    stat("s-err","Помилки","Немає","s");

    h+="</div></div>"; // grid, card

    // Батарея — повна ширина, окремо під сіткою
    if (PM().battEnabled()) {
        int bp=PM().battPercent();
        String battClass=bp>=50?"batt-hi":bp>=20?"batt-mid":"batt-lo";
        h+="<div class='card' id='batt-tile'>";
        h+="<div class='card-title'>🔋 Батарея</div>";
        h+="<div style='display:flex;align-items:center;gap:16px;flex-wrap:wrap'>";
        h+="<div id='s-battv' class='sv gold' style='font-size:24px;min-width:80px'>"+String(PM().battVoltage(),2)+" В</div>";
        h+="<div id='s-battp' class='sv "+String(bp>=50?"g":bp>=20?"y":"r")+"' style='font-size:24px;min-width:60px'>"+String(bp)+"%</div>";
        h+="<div class='batt-bar' style='flex:1;min-width:120px'><div id='batt-fill-d' class='batt-fill "+battClass+"' style='width:"+String(bp)+"%'></div></div>";
        h+="</div></div>";
    } else {
        h+="<div id='batt-tile' style='display:none'></div>";
    }

    h+="<div class='card'><div class='card-title'>🎮 Дії</div><div class='actions'>";
    h+="<a href='/test'       class='btn btn-p'>📨 Тест Telegram</a>";
    h+="<a href='/config'     class='btn btn-s'>⚙️ Налаштування</a>";
    h+="<a href='/logs'       class='btn btn-s'>📋 Журнал</a>";
    h+="<a href='/reboot'     class='btn btn-y' onclick=\"return confirm('Перезавантажити?')\">🔄 Reboot</a>";
    h+="</div></div>";

    h+="<div class='card'><div class='card-title'>📋 Останні події</div>";
    { int n=min(8,Log().count());
      for(int i=0;i<n;i++){
        const EventEntry& e=Log().get(i);
        uint32_t age=(millis()-e.ts)/1000;
        h+="<div class='log-row'><span class='log-ts'>"+Time().duration(age*1000)+" тому</span>"+_esc(e.text)+"</div>";
      }
    }
    h+="</div>";
    h+="</div>"+_foot();
    _srv.send(200,"text/html; charset=utf-8",h);
}

// ============================================================
//  Config page
// ============================================================
void WebUI::_handleConfig(){
    const Config& c=Cfg().cfg;
    String h=_head("Налаштування")+_nav("/config");
    h.reserve(18000);  // велика сторінка — резервуємо щоб уникнути ~15 реалокацій
    h+="<div class='wrap'>";
    if(_srv.hasArg("saved"))
        h+="<div class='alert a-ok'>✅ Налаштування збережено та застосовано.</div>";

    h+="<form method='POST' action='/save'>";

    // ── WiFi ──────────────────────────────────────────────
    h+="<div class='card'><div class='card-title'>📶 WiFi</div>";

    // Scan UI
    h+=R"html(
<div id='scan-wrap'>
  <div style='display:flex;gap:8px;align-items:center;margin-bottom:10px'>
    <span style='font-size:12px;color:#8b949e'>Виберіть мережу зі списку або введіть вручну</span>
    <button type='button' class='btn btn-s btn-sm' id='scan-btn' onclick='doScan()'>📡 Сканувати</button>
  </div>
  <div id='scan-list' style='margin-bottom:14px'></div>
</div>)html";

    h+="<div class='grid2'>";
    // Slot 1
    h+="<div>";
    h+="<div class='fg'><label>SSID 1 — основна</label>"
       "<input type='text' name='s1' id='s1' value='"+_esc(c.wifi_ssid1)+"' placeholder='Назва мережі'></div>";
    h+="<div class='fg'><label>Пароль 1</label>"
       "<input type='password' name='p1' id='p1' value='"+_esc(c.wifi_pass1)+"' placeholder='Пароль' autocomplete='new-password'></div></div>";
    // Slot 2
    h+="<div>";
    h+="<div class='fg'><label>SSID 2 — резервна</label>"
       "<input type='text' name='s2' id='s2' value='"+_esc(c.wifi_ssid2)+"' placeholder='Назва мережі'></div>";
    h+="<div class='fg'><label>Пароль 2</label>"
       "<input type='password' name='p2' id='p2' value='"+_esc(c.wifi_pass2)+"' placeholder='Пароль' autocomplete='new-password'></div></div>";
    h+="</div>"; // grid2

    h+="<div class='alert a-info' style='margin-top:8px;font-size:12px'>"
       "💡 При неможливості підключитися запускається Captive Portal "
       "(<b>"+String(AP_SSID)+"</b> / <b>"+String(AP_PASS)+"</b>). "
       "При активному з'єднанні перепідключення не переривається. Інтервал повтору: <b>3 хв</b>.</div>";

    // Scan JavaScript
    h+=R"js(<script>
async function doScan(){
  const btn=document.getElementById('scan-btn');
  const list=document.getElementById('scan-list');
  btn.disabled=true;
  btn.innerHTML='<span class="spin">⟳</span> Сканування...';
  list.innerHTML='<div style="color:#6e7681;font-size:12px;padding:8px">Пошук мереж...</div>';
  try{
    const nets=await(await fetch('/api/scan')).json();
    if(!nets.length){list.innerHTML='<div class="alert a-warn">Мережі не знайдено</div>';return}
    let html='';
    nets.forEach(n=>{
      const p=Math.max(0,Math.min(100,2*(n.rssi+100)));
      const col=p>=60?'#3fb950':p>=30?'#d29922':'#f85149';
      const enc=n.enc?'🔒':'🔓';
      const saved=n.saved?' saved':'';
      html+=`<div class="wnet${saved}" onclick="fillNet('${n.ssid.replace(/'/g,"\\'")}')">
        <div class="rssi-bar"><div class="rssi-fill" style="width:${p}%;background:${col}"></div></div>
        <div class="wnet-ssid">${n.ssid}${n.saved?' ⭐':''}</div>
        <div class="wnet-info">${n.rssi} dBm ${enc}</div>
      </div>`;
    });
    list.innerHTML=html;
  }catch(e){list.innerHTML='<div class="alert a-err">Помилка сканування</div>'}
  finally{btn.disabled=false;btn.innerHTML='📡 Оновити'}
}
let fillSlot=1;
function fillNet(ssid){
  // Якщо SSID1 вже заповнений і не рівний цьому — заповнюємо SSID2
  const s1=document.getElementById('s1').value;
  const s2=document.getElementById('s2').value;
  if(!s1||s1===ssid){
    document.getElementById('s1').value=ssid;
    document.getElementById('p1').focus();
  } else {
    document.getElementById('s2').value=ssid;
    document.getElementById('p2').focus();
  }
}
// Авто-скан при відкритті сторінки
document.addEventListener('DOMContentLoaded',()=>{
  setTimeout(doScan, 300);
});
</script>)js";

    h+="</div>"; // card WiFi

    // ── Telegram ──────────────────────────────────────────
    h+="<div class='card'><div class='card-title'>🤖 Telegram</div>";
    h+="<div class='fg'><label>Bot Token</label>"
       "<input type='text' name='tok' value='"+_esc(c.bot_token)+"' placeholder='123456:ABCdef...'></div>";
    h+="<div class='grid2'>";
    // Chat ID особистий
    h+="<div class='fg'><label>👤 Chat ID — особистий чат</label>"
       "<input type='text' name='cid' id='cid' value='"+_esc(c.chat_id)+"' placeholder='123456789'>"
       "<span class='hint'>Надішліть боту /start в особистих повідомленнях</span></div>";
    // Chat ID группа
    h+="<div class='fg'><label>👥 Chat ID — група / канал</label>"
       "<input type='text' name='gcid' id='gcid' value='"+_esc(c.group_chat_id)+"' placeholder='-1001234567890'>"
       "<span class='hint'>Додайте бота до групи, він надішле /start</span></div>";
    h+="</div>";
    h+="<div class='grid2' style='margin-top:4px;align-items:end'>";
    h+="<div class='fg'><label>Тип чату (активний)</label><select name='ctype' id='ctype_sel'>";
    h+=_sel(c.chat_type,0,"👤 Особистий чат");
    h+=_sel(c.chat_type,1,"👥 Група / Канал");
    h+="</select><span class='hint'>Визначає який Chat ID використовується для відправки</span></div>";
    h+="<div class='fg'>"
       "<label>&nbsp;</label>"
       "<a href='/getchatid' class='btn btn-s' style='width:100%;box-sizing:border-box'>🔍 Авто-визначення Chat ID</a>"
       "<span class='hint'>Надішліть боту /start, потім натисніть</span>"
       "</div></div>";
    h+="<hr class='divider'><div class='grid3'>";
    h+="<label class='tgl'><input type='checkbox' name='non'"  +_checked(c.notify_power_on) +"><span class='tgl-sl'></span><span class='tgl-txt'>Живлення з'явилося</span></label>";
    h+="<label class='tgl'><input type='checkbox' name='noff'" +_checked(c.notify_power_off)+"><span class='tgl-sl'></span><span class='tgl-txt'>Живлення зникло</span></label>";
    h+="<label class='tgl'><input type='checkbox' name='nboot'"+_checked(c.notify_boot)     +"><span class='tgl-sl'></span><span class='tgl-txt'>Старт пристрою</span></label>";
    h+="</div></div>";

    // ── GPIO живлення ──────────────────────────────────────
    h+="<div class='card'><div class='card-title'>📌 GPIO — моніторинг мережевого живлення</div>";
    h+="<div class='alert a-info' style='margin-bottom:10px;font-size:12px'>📌 Плата: <b>" BOARD_NAME "</b></div>";
    h+="<div class='grid3'>";
    // GPIO пін живлення
    h+="<div class='fg'><label>GPIO пін</label><select name='pin' id='power_pin' onchange='checkGpioConflict()'>";
    for(uint8_t i=0;i<GPIO_INPUT_COUNT;i++){
        uint8_t p=GPIO_INPUT_PINS[i];
        bool inputOnly=false;
        for(uint8_t j=0;j<GPIO_INPUT_ONLY_COUNT;j++) if(GPIO_INPUT_ONLY[j]==p){inputOnly=true;break;}
        String lbl="GPIO "+String(p);
        if(inputOnly)lbl+=" [input-only]";
        h+="<option value='"+String(p)+"'"+(c.power_pin==p?" selected":"")+">"+lbl+"</option>";
    }
    h+="</select>";
#if GPIO_INPUT_ONLY_COUNT > 0
    h+="<span class='hint'>input-only (34,35,36,39) = лише читання</span>";
#else
    h+="<span class='hint'>Всі піни підтримують читання</span>";
#endif
    h+="</div>";
    // Pull mode
    h+="<div class='fg'><label>Підтяжка</label><select name='pmode'>";
    h+=_sel(c.power_pin_mode,0,"INPUT (зовн.)");
    h+=_sel(c.power_pin_mode,1,"INPUT_PULLUP ↑");
    h+=_sel(c.power_pin_mode,2,"INPUT_PULLDOWN ↓");
    h+="</select><span class='hint'>PULLUP/DOWN недоступні для input-only</span></div>";
    // Logic
    h+="<div class='fg'><label>Логіка</label><select name='pinv'>";
    h+="<option value='0'"+String((!c.power_active_low)?" selected":"")+">HIGH = є живлення</option>";
    h+="<option value='1'"+String((c.power_active_low) ?" selected":"")+">LOW  = є живлення</option>";
    h+="</select><span class='hint'>Залежить від схеми</span></div>";
    h+="</div>";
    h+="<div id='gpio-conflict-warn' class='alert a-err' style='display:none;margin-top:8px'>⚠️ Конфлікт: пін GPIO батареї та живлення збігаються!</div>";
    h+="<div class='alert a-warn' style='margin-top:8px;font-size:12px'>⚠️ Макс. напруга на GPIO — <b>3.3 В</b>. Для 5–220 В використовуйте дільник або оптрон.</div>";
    h+="</div>";

    // ── GPIO батарея ──────────────────────────────────────
    h+="<div class='card'><div class='card-title'>🔋 Моніторинг батареї (ADC)</div>";
    // FIXED: added space before id= to avoid checkbox breaking
    h+="<label class='tgl'><input type='checkbox' name='batt_en' id='batt_en_chk'"+_checked(c.batt_enabled)+" onchange='toggleBatt(this.checked)'><span class='tgl-sl'></span><span class='tgl-txt'>Увімкнути моніторинг батареї</span></label>";
    h+="<div id='batt_cfg' style='"+String(c.batt_enabled?"":"display:none")+"'>";
    h+="<div class='grid3'>";
    // ADC pin
    h+="<div class='fg'><label>ADC пін (GPIO)</label><select name='batt_pin' id='batt_pin' onchange='checkGpioConflict()'>";
    for(uint8_t i=0;i<GPIO_ADC_PINS_COUNT;i++){
        uint8_t p=GPIO_ADC_PINS[i];
        h+="<option value='"+String(p)+"'"+(c.batt_pin==p?" selected":"")+">GPIO "+String(p)+"</option>";
    }
    h+="</select><span class='hint'>ADC1 — не конфліктує з WiFi</span></div>";
    // Cell count selector
    h+="<div class='fg'><label>Тип батареї (кількість cells)</label><select name='batt_cells' id='batt_cells' onchange='onCellsChange()'>";
    struct CellOpt{uint8_t n;const char* l;};
    const CellOpt cells[]={{1,"1S — 3.0–4.2В (один елемент)"},{2,"2S — 6.0–8.4В"},{3,"3S — 9.0–12.6В ★"},{4,"4S — 12.0–16.8В"}};
    for(auto& co:cells) h+="<option value='"+String(co.n)+"'"+(c.batt_cells==co.n?" selected":"")+">"+co.l+"</option>";
    h+="</select><span class='hint'>Автоматично встановлює напруги full/empty</span></div>";
    // R1, R2
    h+="<div class='fg'><label>R1 — до батареї (кОм)</label>"
       "<input type='number' name='batt_r1' id='batt_r1' value='"+String(c.batt_r1_kohm,1)+"' step='0.1' min='1' oninput='onR12Change()'>"
       "<span class='hint'>Верхній резистор дільника</span></div>";
    h+="</div>"; // grid3 row1
    h+="<div class='grid3'>";
    h+="<div class='fg'><label>R2 — до GND (кОм)</label>"
       "<input type='number' name='batt_r2' id='batt_r2' value='"+String(c.batt_r2_kohm,1)+"' step='0.1' min='1' oninput='onR12Change()'>"
       "<span class='hint'>Нижній резистор дільника</span></div>";
    // full/empty (read-only preview, submitted as hidden)
    h+="<div class='fg'><label>Напруга «повна» (автo)</label>"
       "<input type='number' name='batt_full' id='batt_full' value='"+String(c.batt_full_v,2)+"' step='0.1' min='1' style='color:#3fb950'>"
       "<span class='hint'>Встановлюється по cells</span></div>";
    h+="<div class='fg'><label>Напруга «порожня» (авто)</label>"
       "<input type='number' name='batt_empty' id='batt_empty' value='"+String(c.batt_empty_v,2)+"' step='0.1' min='1' style='color:#d29922'>"
       "<span class='hint'>Встановлюється по cells</span></div>";
    // V_pin preview
/*    h+="<div class='fg'><label>Розрахунок V_pin @ full</label>"
       "<div id='batt_vpin_calc' style='padding:8px 11px;background:#0d1117;border:1px solid #30363d;border-radius:6px;font-size:13px;color:#58a6ff'></div>"
       "<span class='hint'>Макс. 3.3В на GPIO!</span></div>";  */
    h+="</div>"; // grid3 row2
    h+="<div style='margin-top:8px'><label class='tgl'><input type='checkbox' name='nbatt'"+_checked(c.notify_batt_low)+"><span class='tgl-sl'></span><span class='tgl-txt'>Сповіщення &laquo;батарея &le; 20%&raquo;</span></label></div>";
    h+="</div>"; // batt_cfg
    h+=R"js(<script>
// Li-ion cells: [full_per_cell, empty_per_cell]
const CELLS=[null,[4.2,3.0],[4.2,3.0],[4.2,3.0],[4.2,3.0]];
// Recommended R1/R2 per cell count [R1,R2]
const CELLS_R={1:[10,10],2:[56,27],3:[100,27],4:[150,30]};
function onCellsChange(){
  var n=parseInt(document.getElementById('batt_cells').value)||1;
  if(n<1||n>4)return;
  var full=+(CELLS[n][0]*n).toFixed(2);
  var empty=+(CELLS[n][1]*n).toFixed(2);
  document.getElementById('batt_full').value=full;
  document.getElementById('batt_empty').value=empty;
  var r=CELLS_R[n];
  document.getElementById('batt_r1').value=r[0];
  document.getElementById('batt_r2').value=r[1];
  updateVpinCalc();
}
function onR12Change(){updateVpinCalc();}
function updateVpinCalc(){
  var r1=parseFloat(document.getElementById('batt_r1').value)||0;
  var r2=parseFloat(document.getElementById('batt_r2').value)||0;
  var full=parseFloat(document.getElementById('batt_full').value)||0;
  var el=document.getElementById('batt_vpin_calc');
  if(!el||r1<=0||r2<=0||full<=0){if(el)el.textContent='—';return;}
  var vpin=full*r2/(r1+r2);
  var ok=vpin<=3.3;
  el.textContent='V_pin = '+vpin.toFixed(3)+' В  '+  (ok?'✅ OK':'❌ ПЕРЕВИЩУЄ 3.3В!');
  el.style.color=ok?'#3fb950':'#f85149';
}
function toggleBatt(on){
  document.getElementById('batt_cfg').style.display=on?'':'none';
  checkGpioConflict();
  if(on)updateVpinCalc();
}
function checkGpioConflict(){
  var pp=parseInt(document.getElementById('power_pin')?document.getElementById('power_pin').value:0)||0;
  var bpEl=document.getElementById('batt_pin');
  var bp=bpEl?parseInt(bpEl.value)||0:0;
  var en=document.getElementById('batt_en_chk').checked;
  var w=document.getElementById('gpio-conflict-warn');
  if(w)w.style.display=(en&&pp>0&&pp===bp)?'block':'none';
}
document.addEventListener('DOMContentLoaded',function(){checkGpioConflict();updateVpinCalc();});
</script>)js";
    h+="</div>"; // card


    // ── NTP ──────────────────────────────────────────────
    h+="<div class='card'><div class='card-title'>🕐 Час та часовий пояс</div><div class='grid2'>";
    h+="<div class='fg'><label>NTP сервер</label>"
       "<input type='text' name='ntp' value='"+_esc(c.ntp_server)+"' placeholder='pool.ntp.org'></div>";
    h+="<div class='fg'><label>Часовий пояс</label><select name='tz'>";
    struct TZ{int32_t s;const char* l;};
    const TZ tz[]={
        {-43200,"UTC-12"},{-39600,"UTC-11"},{-36000,"UTC-10"},{-32400,"UTC-9"},
        {-28800,"UTC-8 (LA)"},{-25200,"UTC-7"},{-21600,"UTC-6 (Chicago)"},
        {-18000,"UTC-5 (NY)"},{-14400,"UTC-4"},{-10800,"UTC-3 (Бразилія)"},
        {-7200,"UTC-2"},{-3600,"UTC-1"},{0,"UTC+0 (Лондон)"},
        {3600,"UTC+1 (Берлін)"},{7200,"UTC+2 (Київ) ★"},
        {10800,"UTC+3 (Москва)"},{12600,"UTC+3:30"},{14400,"UTC+4 (Дубай)"},
        {16200,"UTC+4:30"},{18000,"UTC+5"},{19800,"UTC+5:30 (Індія)"},
        {21600,"UTC+6"},{25200,"UTC+7 (Бангкок)"},{28800,"UTC+8 (Пекін)"},
        {32400,"UTC+9 (Токіо)"},{34200,"UTC+9:30"},{36000,"UTC+10 (Сідней)"},
        {39600,"UTC+11"},{43200,"UTC+12"},{46800,"UTC+13"},{50400,"UTC+14"},
    };
    for(auto& t:tz)
        h+="<option value='"+String(t.s)+"'"+(c.tz_offset_sec==t.s?" selected":"")+">"+t.l+"</option>";
    h+="</select></div></div></div>";

    // ── Пристрій ──────────────────────────────────────────
    h+="<div class='card'><div class='card-title'>📛 Пристрій</div><div class='grid2'>";
    h+="<div class='fg'><label>Назва пристрою</label>"
       "<input type='text' name='dname' value='"+_esc(c.device_name)+"'></div>";
    h+="<div class='fg'><label>mDNS ім'я (без .local)</label>"
       "<input type='text' name='mdns' value='"+_esc(c.mdns_name)+"' placeholder='powermon' "
       "pattern='[a-z0-9\\-]+'>"
       "<span class='hint'>Доступ: http://"+_esc(c.mdns_name)+".local</span></div>";
    h+="</div>";
    // Вибір плати
    h+="<div class='fg' style='margin-top:8px'><label>🔲 Модель плати (ESP32)</label><select name='board_type' id='board_type_sel' onchange='onBoardChange()'>";
    struct BrdOpt{uint8_t v;const char* l;const char* hint;};
    const BrdOpt brds[]={
        {0,"ESP32 / WROOM / WROVER / DevKit","GPIO: 0-5,12-27,32-39 · ADC1: 32-39 · LED: GPIO2"},
        {1,"ESP32-C3 SuperMini / XIAO / DevKitM","GPIO: 0-10 · ADC1: 0-4 · LED: GPIO8 (active-low)"},
        {2,"ESP32-S2","GPIO: 1-18,21,33-38 · ADC1: 1-10 · LED: GPIO15"},
        {3,"ESP32-S3 / XIAO-S3","GPIO: 1-18,21,38-48 · ADC1: 1-10 · LED: GPIO48"},
    };
    for(auto& b:brds)
        h+="<option value='"+String(b.v)+"'"+(c.board_type==b.v?" selected":"")+">"+b.l+"</option>";
    h+="</select>";
    h+="<span class='hint' id='board_hint'></span>";
    // GPIO таблиці для JS
    h+=R"js(<script>
const BOARD_GPIO={
  0:{inp:[0,1,2,3,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33,34,35,36,39],
     adc:[32,33,34,35,36,39],inp_only:[34,35,36,39],def_pow:34,def_adc:36,
     hint:'GPIO: 0-5, 12-27, 32-39 | ADC1: 32-39 | input-only: 34,35,36,39'},
  1:{inp:[0,1,2,3,4,5,6,7,8,9,10],
     adc:[0,1,2,3,4],inp_only:[],def_pow:3,def_adc:4,
     hint:'GPIO: 0-10 | ADC1: 0-4 | LED: GPIO8 (active-low)'},
  2:{inp:[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21,33,34,35,36,37,38],
     adc:[1,2,3,4,5,6,7,8,9,10],inp_only:[],def_pow:5,def_adc:6,
     hint:'GPIO: 1-18, 21, 33-38 | ADC1: 1-10'},
  3:{inp:[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21,38,39,40,41,42,45,46,47,48],
     adc:[1,2,3,4,5,6,7,8,9,10],inp_only:[],def_pow:5,def_adc:6,
     hint:'GPIO: 1-18, 21, 38-48 | ADC1: 1-10'},
};
function rebuildSelect(selId,pins,curVal,inputOnly){
  var sel=document.getElementById(selId);if(!sel)return;
  var prev=parseInt(sel.value)||curVal;
  sel.innerHTML='';
  pins.forEach(function(p){
    var opt=document.createElement('option');
    opt.value=p;
    var lbl='GPIO '+p;
    if(inputOnly&&inputOnly.indexOf(p)>=0)lbl+=' [input-only]';
    opt.textContent=lbl;
    if(p===prev)opt.selected=true;
    sel.appendChild(opt);
  });
}
function onBoardChange(){
  var bt=parseInt(document.getElementById('board_type_sel').value)||0;
  var bg=BOARD_GPIO[bt];
  if(!bg)return;
  document.getElementById('board_hint').textContent=bg.hint;
  rebuildSelect('power_pin',bg.inp,bg.def_pow,bg.inp_only);
  rebuildSelect('batt_pin', bg.adc,bg.def_adc,[]);
  checkGpioConflict();
}
document.addEventListener('DOMContentLoaded',function(){
  var bt=parseInt(document.getElementById('board_type_sel').value)||0;
  if(BOARD_GPIO[bt])document.getElementById('board_hint').textContent=BOARD_GPIO[bt].hint;
});
</script>)js";
    h+="</div>";
    h+="</div>";

    // Save bar — дві кнопки: без reboot та з reboot
    h+="<div class='save-bar'>"
       "<button type='submit' name='action' value='save' class='btn btn-g'>💾 Зберегти</button>"
       "<button type='submit' name='action' value='reboot' class='btn btn-p'>🔄 Зберегти і перезавантажити</button>"
       "<a href='/factory' class='btn btn-d' onclick=\"return confirm('Скинути ВСІ налаштування?')\">🗑️ Factory Reset</a>"
       "</div>"
       "<div style='font-size:11px;color:#6e7681;padding-bottom:8px'>💡 Зміна WiFi та mDNS потребує перезавантаження</div>";
    h+="</form></div>"+_foot();
    _srv.send(200,"text/html; charset=utf-8",h);
}

// ============================================================
//  Save config
// ============================================================
void WebUI::_handleSave(){
    Config& c=Cfg().cfg;
#define ARG(dst,key) if(_srv.hasArg(key))strlcpy(dst,_srv.arg(key).c_str(),sizeof(dst))
    ARG(c.wifi_ssid1,"s1"); ARG(c.wifi_pass1,"p1");
    ARG(c.wifi_ssid2,"s2"); ARG(c.wifi_pass2,"p2");
    ARG(c.bot_token,"tok");
    ARG(c.chat_id,"cid");
    ARG(c.group_chat_id,"gcid");
    ARG(c.ntp_server,"ntp"); ARG(c.device_name,"dname"); ARG(c.mdns_name,"mdns");
#undef ARG
    c.notify_power_on  =_srv.hasArg("non");
    c.notify_power_off =_srv.hasArg("noff");
    c.notify_boot      =_srv.hasArg("nboot");
    c.notify_batt_low  =_srv.hasArg("nbatt");
    c.batt_enabled     =_srv.hasArg("batt_en");
    if(_srv.hasArg("ctype"))      c.chat_type        =_srv.arg("ctype").toInt();
    if(_srv.hasArg("pin"))        c.power_pin        =_srv.arg("pin").toInt();
    if(_srv.hasArg("pmode"))      c.power_pin_mode   =_srv.arg("pmode").toInt();
    if(_srv.hasArg("pinv"))       c.power_active_low =(_srv.arg("pinv")=="1");
    if(_srv.hasArg("tz"))         c.tz_offset_sec    =_srv.arg("tz").toInt();
    if(_srv.hasArg("batt_pin"))   c.batt_pin         =_srv.arg("batt_pin").toInt();
    if(_srv.hasArg("batt_r1"))    c.batt_r1_kohm     =_srv.arg("batt_r1").toFloat();
    if(_srv.hasArg("batt_r2"))    c.batt_r2_kohm     =_srv.arg("batt_r2").toFloat();
    if(_srv.hasArg("batt_cells")) c.batt_cells       =_srv.arg("batt_cells").toInt();
    if(_srv.hasArg("batt_full"))  c.batt_full_v      =_srv.arg("batt_full").toFloat();
    if(_srv.hasArg("batt_empty")) c.batt_empty_v     =_srv.arg("batt_empty").toFloat();
    if(_srv.hasArg("board_type")) c.board_type       =_srv.arg("board_type").toInt();
    // Валідація mdns
    for(size_t i=0;i<strlen(c.mdns_name);i++){char& ch=c.mdns_name[i];if(!isalnum(ch)&&ch!='-')ch='-';}

    Cfg().save();
    PM().applyPinConfig();  // застосовуємо GPIO одразу
    WM().restartMDNS();
    Log().add("Налаштування збережено через Web");

    // Якщо натиснуто "Зберегти і перезавантажити" — завжди reboot
    bool forceReboot = (_srv.hasArg("action") && _srv.arg("action")=="reboot");

    if(forceReboot){
        String h=_head("Збереження")+_nav("/config");
        h+="<div class='wrap'><div class='alert a-ok'>✅ Збережено. Перезавантаження...</div>";
        h+="<script>setTimeout(()=>location.href='/',9000)</script></div>"+_foot();
        _srv.send(200,"text/html; charset=utf-8",h);
        delay(300); ESP.restart();
    } else {
        _redirect("/config?saved=1");
    }
}

// ============================================================
//  Logs
// ============================================================
void WebUI::_handleLogs(){
    String h=_head("Журнал")+_nav("/logs");
    h+="<div class='wrap'>";
    h+="<div class='card'><div class='card-title'>📨 Черга Telegram ("+String(Tg().queueCount())+"/"+String(MSG_QUEUE_SIZE)+")</div>";
    if(Tg().queueCount()==0){
        h+="<div style='color:#6e7681;font-size:13px'>Черга порожня ✓</div>";
    }else{
        for(int i=0;i<Tg().queueCount();i++){
            const TgMessage& m=Tg().queueItem(i);
            h+="<div class='q-item "+(m.delivered?String("ok"):String("wait"))+"'>";
            h+="<b>"+(m.delivered?String("✅ Доставлено"):String("⏳ Очікує"))+"</b>";
            h+=" &nbsp;|&nbsp; Спроб: "+String(m.retries)+"/"+String(MSG_MAX_RETRIES);
            h+="<br><code style='font-size:11px;color:#8b949e'>"+_esc(m.text.substring(0,100))+(m.text.length()>100?"…":"")+"</code>";
            h+="</div>";
        }
    }
    h+="</div>";
    h+="<div class='card'><div class='card-title'>📋 Журнал подій ("+String(Log().count())+"/"+String(EVENT_LOG_SIZE)+")</div>";
    { int n=Log().count();
      for(int i=0;i<n;i++){
        const EventEntry& e=Log().get(i);
        uint32_t age=(millis()-e.ts)/1000;
        h+="<div class='log-row'><span class='log-ts'>"+Time().duration(age*1000)+" тому</span>"+_esc(e.text)+"</div>";
      }
    }
    h+="</div></div><script>setTimeout(()=>location.reload(),10000)</script>";
    h+=_foot();
    _srv.send(200,"text/html; charset=utf-8",h);
}

// ============================================================
//  OTA page
// ============================================================
void WebUI::_handleOtaPage(){
    String h=_head("OTA Оновлення")+_nav("/ota");
    h+="<div class='wrap'>";
    h+="<div class='alert a-info'>📋 Версія: <b>" FW_VERSION "</b> &nbsp;|&nbsp; "
       "Heap: <b>"+String(ESP.getFreeHeap()/1024)+" KB</b> &nbsp;|&nbsp; "
       "Flash: <b>"+String(ESP.getFlashChipSize()/1024/1024)+" MB</b></div>";
    h+="<div class='tabs'>"
       "<div class='tab on' onclick='swT(0)'>📁 Файл .bin</div>"
       "<div class='tab'    onclick='swT(1)'>🌐 URL</div></div>";
    h+="<div class='tab-content on' id='tc0'><div class='card'><div class='card-title'>Завантажити .bin файл</div>"
       "<div class='drop' id='drop' onclick='$(\"fw\").click()'>"
       "<div style='font-size:44px;margin-bottom:10px'>📁</div>"
       "<div style='color:#8b949e'>Перетягніть .bin або натисніть для вибору</div>"
       "<div id='fn' style='color:#58a6ff;font-size:13px;margin-top:6px'></div></div>"
       "<input type='file' id='fw' accept='.bin' style='display:none'>"
       "<div id='pw' style='display:none;margin-top:12px'>"
       "<div style='color:#8b949e;font-size:13px'>Завантаження… <span id='pct'>0%</span></div>"
       "<div class='prog'><div class='prog-b' id='pb' style='width:0'></div></div></div>"
       "<div id='res' style='margin-top:12px'></div>"
       "<button class='btn btn-p' id='ubtn' style='display:none;margin-top:12px;width:100%' onclick='doUp()'>🚀 Завантажити прошивку</button>"
       "</div></div>";
    h+="<div class='tab-content' id='tc1'><div class='card'><div class='card-title'>Оновити з URL</div>"
       "<div class='fg'><label>URL прошивки</label>"
       "<input type='url' id='furl' placeholder='http://192.168.1.100/firmware.bin'></div>"
       "<div id='ures' style='margin-top:10px'></div>"
       "<div id='upw' style='display:none;margin-top:10px'>"
       "<div style='color:#8b949e;font-size:13px'>Завантаження з сервера…</div>"
       "<div class='prog'><div class='prog-b' id='upb' style='width:60%;transition:none'></div></div></div>"
       "<button class='btn btn-p' style='margin-top:12px;width:100%' onclick='doUrl()'>🌐 Оновити з URL</button>"
       "</div></div></div>";
    h+=R"js(<script>
const $=id=>document.getElementById(id);
function swT(n){document.querySelectorAll('.tab').forEach((t,i)=>t.className='tab'+(i===n?' on':''));document.querySelectorAll('.tab-content').forEach((t,i)=>t.className='tab-content'+(i===n?' on':''))}
const drop=$('drop'),fi=$('fw');
drop.addEventListener('dragover',e=>{e.preventDefault();drop.classList.add('drag')});
drop.addEventListener('dragleave',()=>drop.classList.remove('drag'));
drop.addEventListener('drop',e=>{e.preventDefault();drop.classList.remove('drag');fi.files=e.dataTransfer.files;showF()});
fi.addEventListener('change',showF);
function showF(){if(!fi.files.length)return;$('fn').textContent='Вибрано: '+fi.files[0].name+' ('+Math.round(fi.files[0].size/1024)+' KB)';$('ubtn').style.display='block'}
function doUp(){
  if(!fi.files.length){alert('Виберіть файл!');return}
  const fd=new FormData();fd.append('firmware',fi.files[0],fi.files[0].name);
  const xhr=new XMLHttpRequest();
  xhr.upload.addEventListener('progress',e=>{if(e.lengthComputable){const p=Math.round(e.loaded/e.total*100);$('pct').textContent=p+'%';$('pb').style.width=p+'%'}});
  xhr.addEventListener('load',()=>{if(xhr.responseText==='OK'){$('res').innerHTML="<div class='alert a-ok'>✅ Готово! Перезавантаження...</div>";setTimeout(()=>location.href='/',7000)}else{$('res').innerHTML="<div class='alert a-err'>❌ "+xhr.responseText+"</div>";$('ubtn').disabled=false}});
  xhr.open('POST','/update');$('pw').style.display='block';$('ubtn').disabled=true;xhr.send(fd);
}
function doUrl(){
  const url=$('furl').value.trim();if(!url){alert('Введіть URL!');return}
  $('upw').style.display='block';$('ures').innerHTML='';
  fetch('/ota-url',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'url='+encodeURIComponent(url)})
    .then(r=>r.text()).then(t=>{$('upw').style.display='none';if(t==='OK'){$('ures').innerHTML="<div class='alert a-ok'>✅ Готово! Перезавантаження...</div>";setTimeout(()=>location.href='/',7000)}else $('ures').innerHTML="<div class='alert a-err'>❌ "+t+"</div>"}).catch(e=>$('ures').innerHTML="<div class='alert a-err'>❌ "+e+"</div>");
}
</script>)js";
    h+=_foot();
    _srv.send(200,"text/html; charset=utf-8",h);
}

void WebUI::_handleOtaUpload(){
    HTTPUpload& up=_srv.upload();
    if(up.status==UPLOAD_FILE_START){Serial.printf("[OTA] %s\n",up.filename.c_str());OTA().handleUploadBegin();}
    else if(up.status==UPLOAD_FILE_WRITE){OTA().handleUploadChunk(up.buf,up.currentSize,false);}
    else if(up.status==UPLOAD_FILE_END){OTA().handleUploadChunk(up.buf,0,true);OTA().handleUploadEnd();}
}
void WebUI::_handleOtaUploadDone(){
    _srv.sendHeader("Connection","close");
    if(OTA().uploadOk()){_srv.send(200,"text/plain","OK");delay(300);ESP.restart();}
    else _srv.send(500,"text/plain",OTA().lastErr());
}
void WebUI::_handleOtaUrl(){
    if(!_srv.hasArg("url")){_srv.send(400,"text/plain","Missing url");return;}
    bool ok=OTA().flashFromUrl(_srv.arg("url"));
    if(ok){_srv.send(200,"text/plain","OK");delay(300);ESP.restart();}
    else _srv.send(500,"text/plain",OTA().lastErr());
}

// ============================================================
//  GetChatId / Test / Reboot / Factory / Captive
// ============================================================
void WebUI::_handleGetChatId(){
    String id;
    String msg;
    bool ok = Tg().fetchChatId(id);
    if(ok){
        // fetchChatId вже зберігає в правильне поле (chat_id або group_chat_id)
        // та встановлює chat_type
        Cfg().save();
        Log().add("Chat ID: "+id+" ("+(Cfg().cfg.chat_type?"група":"особистий")+")");
        msg = "<div class='alert a-ok'>✅ Chat ID визначено: <b>"+id+"</b><br>"
              "Тип: "+(Cfg().cfg.chat_type?"👥 Група/Канал":"👤 Особистий чат")+"</div>";
    }else{
        Log().add("Chat ID: не вдалося. Надішліть боту /start і повторіть.");
        msg = "<div class='alert a-err'>❌ Не вдалося визначити Chat ID.<br>"
              "Надішліть боту <b>/start</b> і спробуйте знову.</div>";
    }
    // Відповідаємо сторінкою з результатом і автоповерненням
    String h=_head("Chat ID")+_nav("/config");
    h+="<div class='wrap'>"+msg;
    h+="<a href='/config' class='btn btn-s' style='margin-top:12px'>← Назад до налаштувань</a>";
    h+="<script>setTimeout(()=>location.href='/config',3000)</script>";
    h+="</div>"+_foot();
    _srv.send(200,"text/html; charset=utf-8",h);
}

void WebUI::_handleSendTest(){
    String msg="🧪 *Тестове повідомлення*\n";
    msg+="📛 "+String(Cfg().cfg.device_name)+"\n";
    msg+="🕐 "+Time().now()+"\n";
    msg+="📶 WiFi: "+WM().ssid()+"\n";
    msg+="⚡ Живлення: "+String(PM().powerPresent()?"✅ Присутнє":"❌ Відсутнє")+"\n";
    if(PM().battEnabled())
        msg+="🔋 Батарея: "+String(PM().battVoltage(),2)+"В ("+String(PM().battPercent())+"%)\n";
    msg+="🔢 v" FW_VERSION;
    Tg().enqueue(msg);
    Log().add("Тестове повідомлення додано до черги");
    _redirect("/");
}

void WebUI::_handleReboot(){
    _srv.send(200,"text/html; charset=utf-8",
        _head("Reboot")+"<div class='wrap'><div class='alert a-warn' style='margin-top:20px'>🔄 Перезавантаження...</div>"
        "<script>setTimeout(()=>location.href='/',8000)</script></div>"+_foot());
    delay(500);ESP.restart();
}
void WebUI::_handleFactoryReset(){
    Cfg().reset();
    _srv.send(200,"text/html; charset=utf-8",
        _head("Factory Reset")+"<div class='wrap'><div class='alert a-err' style='margin-top:20px'>🗑️ Скинуто. Перезавантаження...</div></div>"+_foot());
    delay(500);ESP.restart();
}

void WebUI::_handleCaptive(){
    String h=_head("WiFi Setup")+"<div class='wrap'>";
    h+="<div class='card' style='max-width:500px;margin:0 auto'>";
    h+="<div style='text-align:center;font-size:44px;margin-bottom:10px'>⚡</div>";
    h+="<div class='card-title' style='text-align:center'>Power Monitor — Налаштування WiFi</div>";
    h+="<button type='button' class='btn btn-s' style='width:100%;margin-bottom:12px' onclick='doScan()'>📡 Сканувати мережі</button>";
    h+="<div id='scan-list' style='margin-bottom:12px'></div>";
    h+="<form method='POST' action='/save'>";
    h+="<div class='fg'><label>SSID 1</label><input type='text' name='s1' id='s1' placeholder='Назва мережі'></div>";
    h+="<div class='fg'><label>Пароль 1</label><input type='password' name='p1' id='p1' placeholder='Пароль'></div>";
    h+="<div class='fg'><label>SSID 2 (резервна)</label><input type='text' name='s2' id='s2'></div>";
    h+="<div class='fg'><label>Пароль 2</label><input type='password' name='p2' id='p2'></div>";
    const Config& c=Cfg().cfg;
    auto hi=[&](const char* n,const String& v){h+="<input type='hidden' name='"+String(n)+"' value='"+_esc(v)+"'>";};
    hi("tok",c.bot_token);hi("cid",c.chat_id);hi("gcid",c.group_chat_id);hi("ntp",c.ntp_server);
    hi("dname",c.device_name);hi("mdns",c.mdns_name);
    h+="<input type='hidden' name='ctype' value='"+String(c.chat_type)+"'>";
    h+="<input type='hidden' name='tz'    value='"+String(c.tz_offset_sec)+"'>";
    h+="<input type='hidden' name='pin'   value='"+String(c.power_pin)+"'>";
    h+="<input type='hidden' name='pmode' value='"+String(c.power_pin_mode)+"'>";
    h+="<input type='hidden' name='pinv'  value='"+String(c.power_active_low?1:0)+"'>";
    if(c.notify_power_on) h+="<input type='hidden' name='non'   value='on'>";
    if(c.notify_power_off)h+="<input type='hidden' name='noff'  value='on'>";
    if(c.notify_boot)     h+="<input type='hidden' name='nboot' value='on'>";
    h+="<button type='submit' class='btn btn-p' style='width:100%;margin-top:6px'>💾 Підключитися</button>";
    h+="</form></div></div>";
    // Scan JS для captive
    h+=R"js(<script>
async function doScan(){
  const list=document.getElementById('scan-list');
  list.innerHTML='<div style="color:#6e7681;font-size:12px">Пошук...</div>';
  try{
    const nets=await(await fetch('/api/scan')).json();
    if(!nets.length){list.innerHTML='<div class="alert a-warn">Мережі не знайдено</div>';return}
    let html='';
    nets.forEach(n=>{
      const p=Math.max(0,Math.min(100,2*(n.rssi+100)));
      const col=p>=60?'#3fb950':p>=30?'#d29922':'#f85149';
      html+=`<div class="wnet" onclick="fillNet('${n.ssid.replace(/'/g,"\\'")}')">
        <div class="rssi-bar"><div class="rssi-fill" style="width:${p}%;background:${col}"></div></div>
        <div class="wnet-ssid">${n.ssid}</div>
        <div class="wnet-info">${n.rssi} dBm ${n.enc?'🔒':'🔓'}</div>
      </div>`;
    });
    list.innerHTML=html;
  }catch(e){list.innerHTML='<div class="alert a-err">Помилка</div>'}
}
function fillNet(ssid){
  const s1=document.getElementById('s1').value;
  if(!s1||s1===ssid){document.getElementById('s1').value=ssid;document.getElementById('p1').focus();}
  else{document.getElementById('s2').value=ssid;document.getElementById('p2').focus();}
}
document.addEventListener('DOMContentLoaded',doScan);
</script>)js";
    h+=_foot();
    _srv.send(200,"text/html; charset=utf-8",h);
}

// ============================================================
//  begin
// ============================================================
void WebUI::begin(){
    _srv.on("/",           HTTP_GET,  [this](){_handleRoot();});
    _srv.on("/config",     HTTP_GET,  [this](){_handleConfig();});
    _srv.on("/save",       HTTP_POST, [this](){_handleSave();});
    _srv.on("/api/status", HTTP_GET,  [this](){_handleStatus();});
    _srv.on("/api/scan",   HTTP_GET,  [this](){_handleScan();});
    _srv.on("/logs",       HTTP_GET,  [this](){_handleLogs();});
    _srv.on("/getchatid",  HTTP_GET,  [this](){_handleGetChatId();});
    _srv.on("/test",       HTTP_GET,  [this](){_handleSendTest();});
    _srv.on("/ota",        HTTP_GET,  [this](){_handleOtaPage();});
    _srv.on("/ota-url",    HTTP_POST, [this](){_handleOtaUrl();});
    _srv.on("/reboot",     HTTP_GET,  [this](){_handleReboot();});
    _srv.on("/factory",    HTTP_GET,  [this](){_handleFactoryReset();});
    _srv.on("/update", HTTP_POST,
        [this](){_handleOtaUploadDone();},
        [this](){_handleOtaUpload();}
    );
    _srv.onNotFound([this](){_handleCaptive();});
    _srv.begin();
    Serial.println("[Web] HTTP сервер на порті 80");
}
void WebUI::loop(){_srv.handleClient();}
