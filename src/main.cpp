#include <Arduino.h>

// by mircemk May, 2026

#include <WiFi.h>
#include <WebServer.h>
#include <si5351.h>
#include <Wire.h>
#include "esp_system.h"
#include "config.h"

Si5351 si5351;
unsigned long frequency = 5500000;
const int keySensePin = 18;
const int32_t txOffsetHz = 800; 

WebServer server(80);

const char VFO_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0">
<style>
  :root { 
    /* ТУКА СЕ МЕНУВААТ БОИТЕ - ВЕРЗИЈА V2.4 */
    --panel-bg: #476178;        
    --inner-bezel:#42627d;   
    --lcd-bg: #0077c2;        
    --btn-step: #27ae60;      
    --btn-mode: #2980b9;      
    --btn-mem: #8e44ad;       
    --gold-border: #f1c40f;   
    --text-color: #ecf0f1;    
  }

  * { -webkit-tap-highlight-color: transparent; box-sizing: border-box; -webkit-touch-callout: none; -webkit-user-select: none; user-select: none; }
 body {
  background: #000;
  margin: 0;
  padding: 0;
  min-height: 100vh;
  display: flex;
  justify-content: center;
  font-family: Arial, sans-serif;
  color: var(--text-color);
  overflow: hidden;
}

.vfo-main-frame {
  background: var(--panel-bg);
  width: 100%;
  max-width: 400px;
  height: 100vh;
  display: flex;
  flex-direction: column;
  align-items: center;
  border-left: 2px solid #444;
  border-right: 2px solid #111;
}
 

.display-footer {
  display: flex;
  align-items: center;
  justify-content: space-between;
  border-top: 1px solid rgba(255,255,255,0.2);
  padding-top: 5px;
  margin-bottom: 4px;
  min-width: 0;
}

.vfo-b-readout {
  display: flex;
  align-items: center;
  gap: 5px;
  margin-left: auto;
  white-space: nowrap;
  font-family: Arial, sans-serif;
  font-size: 15px;
  font-weight: bold;
}

 
  
  .bezel-display { background: var(--inner-bezel); width: 92%; margin-top: 15px; padding: 10px; border-radius: 8px; box-shadow: inset 4px 4px 10px #000, 2px 2px 5px rgba(255,255,255,0.1); position: relative; }
  
  .fs-zone { position: absolute; left: 0; top: 0; width: 30%; height: 100%; z-index: 10; cursor: pointer; }
  .mem-zone { position: absolute; left: 30%; top: 0; width: 40%; height: 100%; z-index: 10; cursor: pointer; }
  .reset-zone { position: absolute; right: 0; top: 0; width: 30%; height: 100%; z-index: 10; cursor: pointer; }

  .display { background: var(--lcd-bg); border: 4px solid #111; padding: 10px; box-shadow: inset 0 0 25px #000; height: 125px; display: flex; flex-direction: column; justify-content: space-between; position: relative; transition: background 0.2s; }
  .display.mem-active { background: #e67e22; }
  .display.reset-flash { background: #e74c3c; }

  .display-info { display: flex; justify-content: space-between; font-size: 14px; color: rgba(255,255,255,0.9); font-family: Arial, sans-serif; }
  #f-display { font-size: 55px; font-weight: 900; margin: 0; text-align: right; text-shadow: 2px 2px 4px #000; letter-spacing: -1px; line-height: 1; }
  
  .display-footer { display: flex; align-items: center; border-top: 1px solid rgba(255,255,255,0.2); padding-top: 5px; margin-bottom: 4px; }
  #mode-label, .sig-text { font-size: 15px; font-weight: bold; }
  
  .vfo-b-readout {
  display: flex;
  align-items: center;
  gap: 6px;
  flex-grow: 1;
  justify-content: flex-end;
  margin-left: 25px;
  font-family: Arial, sans-serif;
  font-size: 15px;
  font-weight: bold;
  white-space: nowrap;
}


  .bezel-knob { background: var(--inner-bezel); width: 270px; height: 270px; margin: 25px 0; border-radius: 50%; box-shadow: inset 3px 3px 10px #000, 2px 2px 5px rgba(255,255,255,0.05); display: flex; justify-content: center; align-items: center; }
  #knob { width: 240px; height: 240px; background: conic-gradient(from 0deg, #333, #777 25%, #333 50%, #777 75%, #333); border-radius: 50%; border: 12px solid #1a1a1a; position: relative; will-change: transform; cursor: pointer; box-shadow: 5px 10px 20px #000; }
  #knob::after { content: ''; position: absolute; top: 25px; left: 50%; transform: translateX(-50%); width: 24px; height: 24px; background: #111; border-radius: 50%; box-shadow: inset 2px 2px 5px #000; }

  .controls-container { width: 94%; display: flex; flex-direction: column; }
  .grid { display: grid; gap: 6px; width: 100%; grid-template-columns: repeat(4, 1fr); }
  .group-margin { margin-bottom: 12px; }
  
  .btn { border: 3px solid var(--gold-border); border-radius: 8px; color: #fff; font-weight: 900; font-size: 15px; padding: 11px 0; text-align: center; cursor: pointer; box-shadow: 3px 5px 8px #000; text-transform: uppercase; transition: transform 0.05s; }
  .btn:active { transform: translateY(2px); box-shadow: 1px 2px 4px #000; }
  
  
  .b-step { background: var(--btn-step); font-size: 20px; padding: 12px; grid-column: span 4; }
 
  .b-mem { background: var(--btn-mem); border-color: #555; font-size: 13px; }
    .freq-buttons {
    width: 50%;
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 6px;
    margin-top: 8px;
  }

  .b-freq {
    background: var(--inner-bezel);
    border-color: #555;
    font-size: 24px;
    padding: 11px 0;
    line-height: 1;
   
  }
  .signature { color: #555; font-size: 16px; margin-top: 20px; text-align: center; width: 100%; padding-bottom: 15px; font-weight: normal; }
  /* Keep the interface sized to the phone viewport */
html {
  min-height: 100%;
}

body {
  min-height: 100vh;
  min-height: 100dvh;
  overflow-x: hidden;
  overflow-y: auto;
}

.vfo-main-frame {
  width: 100%;
  max-width: 400px;
  min-height: 100vh;
  min-height: 100dvh;
  height: auto;
  flex-shrink: 0;
  padding-bottom: 12px;
}

/* Keep the display inside the panel */
.bezel-display {
  width: 92%;
}

#f-display {
  font-size: clamp(36px, 13vw, 55px);
}

/* The four controls must be a four-column row */
.control-row {
  display: grid;
  grid-template-columns: repeat(4, minmax(0, 1fr));
  gap: 6px;
  width: 100%;
}

.control-row .btn {
  min-width: 0;
  grid-column: auto;
  padding: 10px 2px;
  font-size: clamp(10px, 3vw, 15px);
}

.control-row .b-step {
  grid-column: auto;
  font-size: clamp(10px, 3vw, 15px);
  padding: 10px 2px;
}


/* Let the dial shrink on narrow screens */
.bezel-knob {
  width: min(270px, 72vw);
  height: min(270px, 72vw);
  margin: 16px 0;
  flex-shrink: 0;
}

#knob {
  width: 89%;
  height: 89%;
}

/* Keep the remaining controls within the panel */
.controls-container {
  width: 94%;
}

.freq-buttons {
  width: 50%;
}

@media (max-height: 700px) {
  .bezel-display {
    margin-top: 8px;
  }

  .bezel-knob {
    margin: 8px 0;
  }

  .signature {
    margin-top: 10px;
  }
}

</style>
</head><body>
  <div class="vfo-main-frame">
    <div class="bezel-display">
      <div class="fs-zone" onclick="toggleFS()"></div>
      <div class="mem-zone" onclick="startMem()"></div>
      <div class="reset-zone" onclick="clearAllMem()"></div>
      
      <div class="display" id="main-display">
        <div class="display-info"><span id="band-label">VFO A</span><span id="step-label">100Hz</span></div>
        <h1 id="f-display">05.500.000</h1>
  <div class="display-footer">
  <span id="mode-label">CW</span>
  <div class="vfo-b-readout">
    <span>VFO B:</span>
    <span id="vfo-b-frequency">000.000</span>
  </div>
</div>

      </div>
    </div>
    
   <div class="freq-buttons">
  <button class="btn b-freq"
          onpointerdown="startAdjustFreq(-1, event)"
          onpointerup="stopAdjustFreq()"
          onpointerleave="stopAdjustFreq()"
          onpointercancel="stopAdjustFreq()"
          oncontextmenu="return false">&lt;</button>

  <button class="btn b-freq"
          onpointerdown="startAdjustFreq(1, event)"
          onpointerup="stopAdjustFreq()"
          onpointerleave="stopAdjustFreq()"
          onpointercancel="stopAdjustFreq()"
          oncontextmenu="return false">&gt;</button>
</div>

<script>
  let freqAdjustTimer = null;
  let freqAdjustDirection = 0;

  function startAdjustFreq(direction, event) {
    event.preventDefault();
    stopAdjustFreq();

    freqAdjustDirection = direction;
    adjustFreq(direction); // Change once immediately

    // Continue changing while held; adjust interval as desired.
    freqAdjustTimer = setInterval(() => {
      adjustFreq(freqAdjustDirection);
    }, 150);
  }

  function stopAdjustFreq() {
    if (freqAdjustTimer !== null) {
      clearInterval(freqAdjustTimer);
      freqAdjustTimer = null;
    }
    freqAdjustDirection = 0;
  }
</script>


    

    <div class="bezel-knob"><div id="knob"></div></div>
    <div class="controls-container">
  <div class="control-row group-margin">
    <button class="btn b-mode" id="mode-btn" onclick="toggleMode()">CW</button>
    <button class="btn b-split inactive" id="split-btn" onclick="toggleSplit()">SPLIT</button>
    <button class="btn b-vfo" id="vfo-btn" onclick="toggleVfo()">VFO A</button>
    <button class="btn b-step" id="step-btn" onclick="nextStep()">100Hz</button>
  </div>

  <div class="grid">
    <div class="btn b-mem" id="m1" onclick="handleMem(1)">M1</div>
    <div class="btn b-mem" id="m2" onclick="handleMem(2)">M2</div>
    <div class="btn b-mem" id="m3" onclick="handleMem(3)">M3</div>
    <div class="btn b-mem" id="m4" onclick="handleMem(4)">M4</div>
  </div>
</div>

    <div class="signature">Si5351 VFO by mircemk</div>
  </div>

  <script>
    var freq = 5500000;
const CW_SSB_THRESHOLD = 5570000; // 5.570 MHz
var curMode = "CW";
var lastAngle = 0;
var curRot = 0;
var isDrag = false;
var lastSent = 0;

    var steps = [10, 100, 1000];
    var stepLabels = ["10Hz", "100Hz", "1KHz"];
    var stepIdx = 1;
    var isMemMode = false;

    function loadSavedMem() {
      for (let i = 1; i <= 4; i++) {
        let saved = localStorage.getItem('vfo_m' + i);
        if (saved) {
          let relativekHz = (parseInt(saved) - 5500000) / 1000;
          let displayString = relativekHz.toFixed(3);
          let parts = displayString.split('.');
          
          // SAFE WAY: Pad the memory slots safely without loops
          let leadingDigits = parts[0].padStart(3, '0');
          
          document.getElementById('m' + i).innerText = leadingDigits + '.' + parts[1];
        } else {
          document.getElementById('m' + i).innerText = "M" + i;
        }
      }
    }

    function clearAllMem() {
      for(let i=1; i<=4; i++) localStorage.removeItem('vfo_m'+i);
      loadSavedMem();
      let d = document.getElementById('main-display');
      d.classList.add('reset-flash');
      setTimeout(() => d.classList.remove('reset-flash'), 300);
    }

    function startMem() {
      isMemMode = true;
      document.getElementById('main-display').classList.add('mem-active');
      document.querySelectorAll('.b-mem').forEach(b => b.classList.add('save-ready'));
    }

    function handleMem(id) {
      if (isMemMode) {
        localStorage.setItem('vfo_m' + id, freq);
        
        let relativekHz = (freq - 5500000) / 1000;
        let displayString = relativekHz.toFixed(3);
        let parts = displayString.split('.');
        
        // SAFE WAY: Pad the saved memory slot safely
        let leadingDigits = parts[0].padStart(3, '0');
        
        document.getElementById('m' + id).innerText = leadingDigits + '.' + parts[1];
        isMemMode = false;
        document.getElementById('main-display').classList.remove('mem-active');
        document.querySelectorAll('.b-mem').forEach(b => b.classList.remove('save-ready'));
      } else {
        let saved = localStorage.getItem('vfo_m' + id);
        if (saved) { 
          freq = parseInt(saved); 
          updateUI(); 
          sendFreq(); 
        }
      }
    }

    function toggleFS() {
      var d = document.documentElement;
      if(!document.fullscreenElement) d.requestFullscreen().catch(e=>{});
      else document.exitFullscreen();
    }

    function updateMode() {
  curMode = (freq < CW_SSB_THRESHOLD) ? "CW" : "SSB";
}

    
   function updateUI() {
      updateMode();
      let relativekHz = (freq - 5500000) / 1000;
      let displayString = relativekHz.toFixed(3);
      let parts = displayString.split('.');
      
      // SAFE WAY: Pad the front digits to 3 spots using built-in padStart
      let leadingDigits = parts[0].padStart(3, '0');

      document.getElementById('f-display').innerText = leadingDigits + '.' + parts[1];
      document.getElementById('mode-label').innerText = curMode;
    }
    
    function nextStep() {
      stepIdx = (stepIdx + 1) % steps.length;
      document.getElementById('step-btn').innerText = stepLabels[stepIdx];
      document.getElementById('step-label').innerText = stepLabels[stepIdx];
    }    
    
    function adjustFreq(direction) {
      // Change frequency by the currently selected step
      freq += direction * steps[stepIdx];

      // Limit the Si5351 range to 5.500 MHz–6.000 MHz
      if (freq < 5500000) freq = 5500000;
      if (freq > 6000000) freq = 6000000;

      updateUI();
      sendFreq();
    }


    function sendFreq() {
      let now = Date.now();
      if (now - lastSent > 50) { fetch('/set?f=' + freq); lastSent = now; }
    }
    function getAngle(x, y) {
      let r = document.getElementById('knob').getBoundingClientRect();
      return Math.atan2(y - (r.top + r.height/2), x - (r.left + r.width/2)) * 180 / Math.PI;
    }
       function move(e) {
      if (!isDrag) return;
      let ev = e.touches ? e.touches[0] : e;
      let ang = getAngle(ev.clientX, ev.clientY);
      let d = ang - lastAngle;
      if (d > 180) d -= 360; if (d < -180) d += 360;
      
      // 1. Calculate what the next frequency would be
      let nextFreq = freq + (Math.round(d) * (steps[stepIdx] / 10));
      
      // 2. Strict clamping limits for the TS-830S EXT VFO window
      if (nextFreq < 5500000) nextFreq = 5500000; // Hard bottom stop at 000.000
      if (nextFreq > 6000000) nextFreq = 6000000; // Hard top stop at 500.000
      
      // 3. Only rotate the wheel and send data if we are actually within bounds
      if (nextFreq !== freq) {
        freq = nextFreq;
        curRot += d;
        document.getElementById('knob').style.transform = 'rotate(' + curRot + 'deg)';
        updateUI();
        sendFreq();
      }
      
      lastAngle = ang;
    }

    let knob = document.getElementById('knob');
    knob.addEventListener('mousedown', function(e) { isDrag = true; lastAngle = getAngle(e.clientX, e.clientY); });
    knob.addEventListener('touchstart', function(e) { isDrag = true; lastAngle = getAngle(e.touches[0].clientX, e.touches[0].clientY); e.preventDefault(); }, {passive: false});
    window.addEventListener('mouseup', () => isDrag = false);
    window.addEventListener('touchend', () => isDrag = false);
    window.addEventListener('mousemove', move);
    window.addEventListener('touchmove', move, {passive: false});
    
    

    loadSavedMem();
    updateUI();
  </script>
</body></html>
)rawliteral";

//void updateFrequency(unsigned long f) { si5351.set_freq(f * 100ULL, SI5351_CLK0); }
const unsigned long CW_SSB_THRESHOLD_HZ = 5570000UL;

void updateFrequency(unsigned long f) {
  uint64_t outputHz = f;

  // Apply the TX offset only below 5.570 MHz, and only while transmitting.
  if (f < CW_SSB_THRESHOLD_HZ &&
      digitalRead(keySensePin) == LOW) {
    outputHz += txOffsetHz;
  }

  // Etherkit set_freq() expects units of 0.01 Hz.
  si5351.set_freq(outputHz * 100ULL, SI5351_CLK0);
}


void handleRoot() {
  server.send_P(200, "text/html", VFO_HTML);
  
}

void handleSet() {
  if (!server.hasArg("f")) {
    server.send(400, "text/plain", "Missing frequency");
    return;
  }

  unsigned long newFrequency = server.arg("f").toInt();

  // Basic safety range: 100 kHz to 30 MHz
  if (newFrequency < 100000 || newFrequency > 30000000) {
    server.send(400, "text/plain", "Invalid frequency");
    return;
  }

  frequency = newFrequency;
  updateFrequency(frequency);

  Serial.print("Frequency set to: ");
  Serial.println(frequency);

  server.send(200, "text/plain", "OK");
}

void handleSignal() {
  // Temporary test value.
  // Replace with analogRead() after the VFO is stable.
  server.send(200, "text/plain", "0");
}
void checkKeySense() {
  static int lastState = HIGH;
  static unsigned long lastChange = 0;
  uint64_t outputHz = frequency;
  int state = digitalRead(keySensePin);

  if (state != lastState) {
    lastState = state;
    lastChange = millis();
  }

  if (millis() - lastChange >= 30) {
    static int appliedState = HIGH;

    if (state == LOW && frequency < CW_SSB_THRESHOLD_HZ) {
  outputHz += txOffsetHz;
}

Serial.printf("Key sense: %s; mode: %s; VFO: %lu Hz; output: %llu Hz\n",
              state == LOW ? "TX" : "RX",
              frequency < CW_SSB_THRESHOLD_HZ ? "CW" : "SSB",
              frequency,
              outputHz);

      updateFrequency(frequency);
    }
 }



void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(keySensePin,INPUT_PULLUP);
  Wire.begin(21, 22);
  Wire.setClock(100000);

if (!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0)) {
  Serial.println("Si5351 initialization failed");
} else {
  Serial.println("Si5351 initialized");
  si5351.set_correction(3542, SI5351_PLL_INPUT_XO);
  si5351.drive_strength(SI5351_CLK0,SI5351_DRIVE_4MA);
  //si5351.set_freq(550000000ULL, SI5351_CLK0);  //5.5mhz
  updateFrequency(frequency);  // Starts at 5.500 MHz, with no TX offset
  si5351.output_enable(SI5351_CLK0, 1);
}
WiFi.mode(WIFI_STA);
WiFi.setSleep(false);

WiFi.begin(WIFI_SSID, WIFI_PASS);

while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

Serial.println();
Serial.print("Connected. IP address: ");
Serial.println(WiFi.localIP());



  // Register web-server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/getS", HTTP_GET, handleSignal);
  server.begin();
  


}


void loop() {
  server.handleClient();
  checkKeySense();
  delay(1);
}

