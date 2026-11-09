const $ = (id) => document.getElementById(id);
const API = "http://127.0.0.1:8080";
let steps = [];
let stepIndex = 0;
let playTimer = null;
let latestResult = null;

function toast(message) {
  const el = $("toast");
  el.textContent = message;
  el.classList.add("show");
  clearTimeout(toast.timer);
  toast.timer = setTimeout(() => el.classList.remove("show"), 3200);
}
function parseValues() {
  const raw = $("values").value.trim();
  if (!raw) throw new Error("Please enter some numbers.");
  const parts = raw.split(",");
  if (parts.length > 100) throw new Error("Please use 100 numbers or fewer.");
  const vals = parts.map(p => {
    if (p.trim() === "" || !/^-?\d+$/.test(p.trim())) throw new Error("Use whole numbers separated by commas.");
    const n = Number(p.trim());
    if (!Number.isSafeInteger(n)) throw new Error("One of the numbers is too large.");
    return n;
  });
  return vals;
}
async function checkConnection() {
  try {
    const response = await fetch(API + "/health");
    const data = await response.json();
    if (!data.ok) throw new Error("not connected");
    $("connectionText").textContent = "Connected on port 8080";
    $("connectionText").style.color = "#55d6a5";
    document.querySelector(".status-dot").classList.add("online");
  } catch {
    $("connectionText").textContent = "Start server.exe first";
    $("connectionText").style.color = "#f0b85a";
    document.querySelector(".status-dot").classList.remove("online");
  }
}
function renderBars(values, active = -1) {
  const chart = $("barChart");
  chart.innerHTML = "";
  if (!values || !values.length) {
    chart.innerHTML = '<div class="empty-state"><strong>No values to display</strong></div>';
    return;
  }
  const maxAbs = Math.max(1, ...values.map(v => Math.abs(v)));
  values.forEach((value, index) => {
    const bar = document.createElement("div");
    bar.className = "bar" + (index === active ? " highlight" : "");
    bar.style.height = Math.max(5, Math.abs(value) / maxAbs * 78) + "%";
    const label = document.createElement("span");
    label.textContent = value;
    bar.appendChild(label);
    bar.title = `Index ${index}: ${value}`;
    chart.appendChild(bar);
  });
}
function renderStep() {
  if (!steps.length) return;
  stepIndex = Math.max(0, Math.min(stepIndex, steps.length - 1));
  renderBars(steps[stepIndex]);
  $("stepCount").textContent = `Step ${stepIndex + 1} of ${steps.length}`;
  $("prevBtn").disabled = stepIndex === 0;
  $("nextBtn").disabled = stepIndex >= steps.length - 1;
}
function stopPlayback() {
  if (playTimer) clearInterval(playTimer);
  playTimer = null;
  $("playBtn").textContent = "▶ Play";
}
function togglePlay() {
  if (playTimer) { stopPlayback(); return; }
  if (!steps.length) { toast("Run an algorithm first."); return; }
  if (stepIndex >= steps.length - 1) stepIndex = 0;
  $("playBtn").textContent = "Ⅱ Pause";
  playTimer = setInterval(() => {
    if (stepIndex >= steps.length - 1) { stopPlayback(); return; }
    stepIndex++;
    renderStep();
  }, 650);
}
async function runAlgorithm() {
  stopPlayback();
  let values;
  try { values = parseValues(); }
  catch (err) { toast(err.message); return; }
  const algorithm = $("algorithm").value;
  const target = Number($("target").value || 0);
  $("runBtn").disabled = true;
  $("runBtn").textContent = "Running…";
  try {
    const response = await fetch(API + "/run", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ algorithm, values, target: String(target) })
    });
    const data = await response.json();
    if (!response.ok || !data.ok) throw new Error(data.error || "The C++ server returned an error.");
    latestResult = data;
    steps = data.steps.length ? data.steps : [data.values];
    stepIndex = 0;
    renderStep();
    $("complexity").textContent = data.complexity;
    $("complexityDetail").textContent = data.complexity;
    $("comparisons").textContent = data.comparisons;
    $("swaps").textContent = data.swaps;
    $("runtime").textContent = data.milliseconds.toFixed(3);
    $("insightTitle").textContent = ({
      bubble: "Bubble Sort",
      selection: "Selection Sort",
      insertion: "Insertion Sort",
      linear: "Linear Search",
      binary: "Binary Search"
    })[algorithm];
    $("explanation").textContent = data.explanation;
    $("spaceDetail").textContent = algorithm === "insertion" ? "O(1) auxiliary" : algorithm === "binary" ? "O(1) auxiliary" : "O(1) auxiliary";
    $("searchResult").textContent = "";
    if (algorithm === "linear" || algorithm === "binary") {
      $("searchResult").textContent = data.foundIndex >= 0
        ? `Target ${target} found at index ${data.foundIndex}${algorithm === "binary" ? " in the sorted array" : ""}.`
        : `Target ${target} was not found.`;
    }
    toast("Algorithm completed in the C++ engine.");
  } catch (err) {
    toast("Can't reach the C++ server. Run start-server.bat and keep its window open.");
    console.error(err);
  } finally {
    $("runBtn").disabled = false;
    $("runBtn").textContent = "▶ Run algorithm";
  }
}
$("runBtn").addEventListener("click", runAlgorithm);
$("randomBtn").addEventListener("click", () => {
  const values = Array.from({length: 8}, () => Math.floor(Math.random() * 90) + 5);
  $("values").value = values.join(", ");
  toast("Random array generated.");
});
$("prevBtn").addEventListener("click", () => { stopPlayback(); stepIndex--; renderStep(); });
$("nextBtn").addEventListener("click", () => { stopPlayback(); stepIndex++; renderStep(); });
$("playBtn").addEventListener("click", togglePlay);
$("algorithm").addEventListener("change", () => {
  const searchMode = ["linear", "binary"].includes($("algorithm").value);
  $("targetField").style.opacity = searchMode ? "1" : ".45";
  $("target").disabled = !searchMode;
});
$("algorithm").dispatchEvent(new Event("change"));
checkConnection();
setInterval(checkConnection, 8000);
