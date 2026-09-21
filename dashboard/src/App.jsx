import { useEffect, useState } from "react";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
} from "recharts";
import "./App.css";

// Reduce a large dataset to a manageable number of points for chart display
function downsampleData(data, maxPoints = 200) {
  if (data.length <= maxPoints) {
    return data;
  }

  const bucketSize = data.length / maxPoints;
  const result = [];

  for (let i = 0; i < maxPoints; i++) {
    const start = Math.floor(i * bucketSize);
    const end = Math.floor((i + 1) * bucketSize);

    const bucket = data.slice(start, end);

    if (bucket.length === 0) {
      continue;
    }

    const avgTemperature =
      bucket.reduce(
        (sum, item) => sum + Number(item.temperature || 0),
        0
      ) / bucket.length;

    const avgHumidity =
      bucket.reduce(
        (sum, item) => sum + Number(item.humidity || 0),
        0
      ) / bucket.length;

    const avgLight =
      bucket.reduce(
        (sum, item) => sum + Number(item.light || 0),
        0
      ) / bucket.length;

    const middleItem = bucket[Math.floor(bucket.length / 2)];

    result.push({
      ...middleItem,
      temperature: Number(avgTemperature.toFixed(2)),
      humidity: Number(avgHumidity.toFixed(2)),
      light: Math.round(avgLight),
    });
  }

  return result;
}

function App() {
  const [sensorData, setSensorData] = useState(null);
  const [historyData, setHistoryData] = useState([]);
  const [timeRange, setTimeRange] = useState(1);
  const [error, setError] = useState(null);
  const [mode, setMode] = useState("AUTO");
  const [ledState, setLedState] = useState("OFF");
  const [controlMessage, setControlMessage] = useState("");
  const [controlLoading, setControlLoading] = useState(false);
  const [deviceStatus, setDeviceStatus] = useState({
    online: false,
    mode: "UNKNOWN",
    led: "UNKNOWN",
    light: 0,
    packet: 0,
    total_packets: 0,
    missing_packets: 0,
  });

  const sendModeCommand = async (newMode) => {
    setControlLoading(true);
    setControlMessage("");
    try {
      const response = await fetch(
        `http://localhost:8000/api/device/mode/${newMode}`,
        {
          method: "POST",
        }
      );
      const result = await response.json();
      if (!response.ok || !result.success) {
        throw new Error(result.message || "Failed to change mode");
      }
      setMode(newMode);
      setControlMessage(`Mode changed to ${newMode}`);
    } catch (error) {
      console.error(error);
      setControlMessage("Failed to change mode");
    } finally {
      setControlLoading(false);
    }
  };

  const sendLedCommand = async (state) => {
    setControlLoading(true);
    setControlMessage("");
    try {
      const response = await fetch(
        `http://localhost:8000/api/device/led/${state}`,
        {
          method: "POST",
        }
      );
      const result = await response.json();
      if (!response.ok || !result.success) {
        throw new Error(result.message || "Failed to control LED");
      }
      setLedState(state);
      setControlMessage(`LED turned ${state}`);
    } catch (error) {
      console.error(error);
      setControlMessage("Failed to control LED");
    } finally {
      setControlLoading(false);
    }
  };

  const fetchDeviceStatus = async () => {
    try {
      const response = await fetch("http://localhost:8000/api/device/status");
      if (!response.ok) {
        throw new Error("Failed to fetch device status");
      }
      const status = await response.json();
      setDeviceStatus(status);
      setMode(status.mode);
      setLedState(status.led);
    } catch (error) {
      console.error("Device status error:", error);
    }
  };

  useEffect(() => {
    const fetchSensorData = async () => {
      try {
        // Get latest sensor reading
        const latestResponse = await fetch(
          "http://localhost:8000/api/sensors/latest"
        );
        if (!latestResponse.ok) {
          throw new Error("Failed to fetch latest sensor data");
        }
        const latestData = await latestResponse.json();
        setSensorData(latestData);
        setError(null);

        // Get recent history
        const historyResponse = await fetch(
          `http://localhost:8000/api/sensors/history?hours=${timeRange}`
        );
        if (!historyResponse.ok) {
          throw new Error("Failed to fetch sensor history");
        }
        const history = await historyResponse.json();
        const formattedData = history.map((item) => ({
          ...item,
          timestampMs: new Date(item.timestamp).getTime(),
        }));

        const chartData = downsampleData(formattedData, 200);
        setHistoryData(chartData);

      } catch (error) {
        console.error(error);
        setError("Unable to connect to FastAPI");
      }
    };

    // Fetch immediately when dashboard loads
    fetchSensorData();
    fetchDeviceStatus();

    // Fetch new data every 4 seconds
    const interval = setInterval(() => {
      fetchSensorData();
      fetchDeviceStatus();
    }, 4000);

    // Stop timer when page is closed
    return () => {
      clearInterval(interval);
    };
  }, [timeRange]);
    // Calculate a readable temperature Y-axis range
  const temperatureValues = historyData
    .map((item) => Number(item.temperature))
    .filter((value) => Number.isFinite(value));

  let temperatureMin = 20;
  let temperatureMax = 35;

  if (temperatureValues.length > 0) {
    const dataMin = Math.min(...temperatureValues);
    const dataMax = Math.max(...temperatureValues);

    const dataRange = dataMax - dataMin;

    // Always keep at least 2°C padding
    const padding = Math.max(2, dataRange * 0.25);

    temperatureMin = Math.floor(dataMin - padding);
    temperatureMax = Math.ceil(dataMax + padding);
  }

  return (
    <div className="dashboard">
      {/* Header */}
      <header className="dashboard-header">
        <div className="header-title">
          <div className="header-icon">📈</div>
          <div>
            <h1>IoT Sensor Dashboard</h1>
            <p>Real-time monitoring & device control</p>
          </div>
        </div>
        <div
          className={`connection-status ${
            deviceStatus.online ? "connection-online" : "connection-offline"
          }`}
        >
          <span
            className={`status-dot ${
              deviceStatus.online ? "online" : "offline"
            }`}
          ></span>
          <div>
            <strong>
              {deviceStatus.online ? "Gateway Online" : "Gateway Offline"}
            </strong>
            <small>
              {deviceStatus.online
                ? "System is running normally"
                : "Connection lost"}
            </small>
          </div>
        </div>
      </header>

      {error && <div className="error-message">{error}</div>}

      {sensorData && (
        <>
          {/* Sensor Cards */}
          <div className="sensor-grid">
            {/* Temperature */}
            <div className="sensor-card temperature-card">
              <div className="sensor-card-top">
                <div className="card-icon">🌡️</div>
                <span className="sensor-label">TEMPERATURE</span>
              </div>
              <div className="sensor-value">
                {sensorData.temperature} <span>°C</span>
              </div>
              <div className="sensor-status">
                <span className="indicator-dot"></span> Live reading
              </div>
            </div>

            {/* Humidity */}
            <div className="sensor-card humidity-card">
              <div className="sensor-card-top">
                <div className="card-icon">💧</div>
                <span className="sensor-label">HUMIDITY</span>
              </div>
              <div className="sensor-value">
                {sensorData.humidity} <span>%</span>
              </div>
              <div className="sensor-status">
                <span className="indicator-dot"></span> Live reading
              </div>
            </div>

            {/* Light */}
            <div className="sensor-card light-card">
              <div className="sensor-card-top">
                <div className="card-icon">☀️</div>
                <span className="sensor-label">LIGHT LEVEL</span>
              </div>
              <div className="sensor-value">
                {sensorData.light} <span> ADC</span>
              </div>
              <div className="sensor-status">
                <span
                  className={`indicator-dot ${
                    deviceStatus.online ? "live" : "stale"
                  }`}
                ></span>
                {deviceStatus.online ? "Live reading" : "Last known reading"}
              </div>
            </div>
          </div>

          {/* System Status */}
          <div className="system-status-card">
            <div className="section-header">
              <div>
                <h2>System Status</h2>
                <p>Real-time gateway and communication information</p>
              </div>
              <div
                className={`system-badge ${
                  deviceStatus.online ? "online" : "offline"
                }`}
              >
                <span className="status-dot-small"></span>
                {deviceStatus.online ? "SYSTEM ONLINE" : "SYSTEM OFFLINE"}
              </div>
            </div>
            <div className="system-status-grid">
              {/* Gateway */}
              <div className="status-item">
                <div className="status-item-icon">📡</div>
                <div className="status-item-content">
                  <span>Gateway</span>
                  <strong
                    className={
                      deviceStatus.online ? "status-online" : "status-offline"
                    }
                  >
                    {deviceStatus.online ? "ONLINE" : "OFFLINE"}
                  </strong>
                </div>
                <span
                  className={`status-indicator ${
                    deviceStatus.online
                      ? "indicator-online"
                      : "indicator-offline"
                  }`}
                ></span>
              </div>

              {/* Operating Mode */}
              <div className="status-item">
                <div className="status-item-icon">⚙️</div>
                <div className="status-item-content">
                  <span>Operating Mode</span>
                  <strong>{deviceStatus.mode}</strong>
                </div>
                <span
                  className={`mode-badge ${
                    deviceStatus.mode === "AUTO" ? "mode-auto" : "mode-manual"
                  }`}
                >
                  {deviceStatus.mode}
                </span>
              </div>

              {/* LED */}
              <div className="status-item">
                <div className="status-item-icon">💡</div>
                <div className="status-item-content">
                  <span>LED Status</span>
                  <strong>{deviceStatus.led}</strong>
                </div>
                <span
                  className={`led-indicator ${
                    deviceStatus.led === "ON"
                      ? "led-on-indicator"
                      : "led-off-indicator"
                  }`}
                ></span>
              </div>

              {/* Last Packet */}
              <div className="status-item">
                <div className="status-item-icon">📦</div>
                <div className="status-item-content">
                  <span>Last Packet</span>
                  <strong>#{deviceStatus.packet}</strong>
                </div>
              </div>

              {/* Total Packets */}
              <div className="status-item">
                <div className="status-item-icon">📊</div>
                <div className="status-item-content">
                  <span>Total Packets</span>
                  <strong>{deviceStatus.total_packets}</strong>
                </div>
              </div>

              {/* Missing Packets */}
              <div className="status-item">
                <div className="status-item-icon">⚠️</div>
                <div className="status-item-content">
                  <span>Missing Packets</span>
                  <strong
                    className={
                      deviceStatus.missing_packets > 0
                        ? "status-warning"
                        : "status-online"
                    }
                  >
                    {deviceStatus.missing_packets}
                  </strong>
                </div>
              </div>
            </div>
          </div>

          {/* Temperature Chart */}
          {/* Temperature Chart */}
          <div className="chart-card">
            <div className="chart-header">
              <div>
                <h2>Temperature History</h2>
                <p className="chart-subtitle">
                  Temperature readings over time
                </p>
              </div>

              <div className="time-range-buttons">
                <button
                  className={timeRange === 1 ? "active" : ""}
                  onClick={() => setTimeRange(1)}
                >
                  1H
                </button>

                <button
                  className={timeRange === 6 ? "active" : ""}
                  onClick={() => setTimeRange(6)}
                >
                  6H
                </button>

                <button
                  className={timeRange === 24 ? "active" : ""}
                  onClick={() => setTimeRange(24)}
                >
                  24H
                </button>

                <button
                  className={timeRange === 0 ? "active" : ""}
                  onClick={() => setTimeRange(0)}
                >
                  All
                </button>
              </div>
            </div>

            <ResponsiveContainer width="100%" height={320}>
              <LineChart
                data={historyData}
                margin={{ top: 10, right: 20, left: 0, bottom: 10 }}
              >
                <CartesianGrid strokeDasharray="3 3" />

                <XAxis
                    type="number"
                    dataKey="timestampMs"
                    domain={["dataMin", "dataMax"]}
                    tick={{ fontSize: 12 }}
                    minTickGap={50}
                    tickFormatter={(value) =>
                      new Date(value).toLocaleTimeString([], {
                        hour: "2-digit",
                        minute: "2-digit",
                      })
                    }
                  />

                <YAxis
                  domain={[temperatureMin, temperatureMax]}
                  tick={{ fontSize: 12 }}
                  tickCount={7}
                />

                <Tooltip
                    labelFormatter={(value) =>
                      new Date(value).toLocaleString([], {
                        month: "short",
                        day: "numeric",
                        hour: "2-digit",
                        minute: "2-digit",
                        second: "2-digit",
                      })
                    }
                    formatter={(value, name) => {
                      const units = {
                        temperature: "°C",
                        humidity: "%",
                        light: "",
                      };

                      return [
                        `${value}${units[name] || ""}`,
                        name.charAt(0).toUpperCase() + name.slice(1),
                      ];
                    }}
                  />

                <Line
                  type="monotone"
                  dataKey="temperature"
                  stroke="#ff7300"
                  strokeWidth={3}
                  dot={false}
                  activeDot={{ r: 5 }}
                />
              </LineChart>
            </ResponsiveContainer>
          </div>

          {/* Humidity Chart */}
          <div className="chart-card">
            <h2>Humidity History</h2>
            <p className="chart-subtitle humidity-subtitle">
                  Humidity sensor readings over time
            </p>
            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={historyData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis
                    type="number"
                    dataKey="timestampMs"
                    domain={["dataMin", "dataMax"]}
                    tick={{ fontSize: 12 }}
                    minTickGap={50}
                    tickFormatter={(value) =>
                      new Date(value).toLocaleTimeString([], {
                        hour: "2-digit",
                        minute: "2-digit",
                      })
                    }
                  />
                <YAxis />
                <Tooltip
                    labelFormatter={(value) =>
                      new Date(value).toLocaleString([], {
                        month: "short",
                        day: "numeric",
                        hour: "2-digit",
                        minute: "2-digit",
                        second: "2-digit",
                      })
                    }
                  />
                <Line
                  type="monotone"
                  dataKey="humidity"
                  stroke="#0088fe"
                  strokeWidth={2}
                  dot={false}
                />
              </LineChart>
            </ResponsiveContainer>
          </div>

          {/* Light Chart */}
          <div className="chart-card">
            <div className="chart-header">
              <div>
                <h2>Light Level History</h2>
                <p className="chart-subtitle">
                  Light sensor readings over time
                </p>
              </div>
            </div>

            <ResponsiveContainer width="100%" height={300}>
              <LineChart
                data={historyData}
                margin={{ top: 10, right: 20, left: 10, bottom: 10 }}
              >
                <CartesianGrid strokeDasharray="3 3" />

                <XAxis
                  type="number"
                  dataKey="timestampMs"
                  domain={["dataMin", "dataMax"]}
                  tick={{ fontSize: 12 }}
                  minTickGap={50}
                  tickFormatter={(value) =>
                    new Date(value).toLocaleTimeString([], {
                      hour: "2-digit",
                      minute: "2-digit",
                    })
                  }
                />

                <YAxis
                  type="number"
                  tick={{ fontSize: 12 }}
                  width={60}
                  domain={["auto", "auto"]}
                  tickCount={6}
                  tickFormatter={(value) => Math.round(value)}
                />

                <Tooltip
                  labelFormatter={(value) =>
                    new Date(value).toLocaleString([], {
                      month: "short",
                      day: "numeric",
                      hour: "2-digit",
                      minute: "2-digit",
                      second: "2-digit",
                    })
                  }
                  formatter={(value) => [
                    `${Math.round(value)} ADC`,
                    "Light Level",
                  ]}
                />

                <Line
                  type="monotone"
                  dataKey="light"
                  stroke="#82ca9d"
                  strokeWidth={2}
                  dot={false}
                  activeDot={{ r: 5 }}
                />
              </LineChart>
            </ResponsiveContainer>
          </div>

          {/* Device Information */}
          <div className="info-card">
            <h2>Device Information</h2>
            <div className="info-grid">
              <div>
                <span>Node ID</span>
                <strong>{sensorData.node}</strong>
              </div>
              <div>
                <span>Packet ID</span>
                <strong>{sensorData.packet}</strong>
              </div>
              <div>
                <span>Last Updated</span>
                <strong>
                  {new Date(sensorData.timestamp).toLocaleString()}
                </strong>
              </div>
            </div>
          </div>

          {/* Device Control */}
          <div className="control-card">
            <h2>Device Control</h2>
            <div className="device-status">
              <div>
                <span>Current Mode</span>
                <strong>{mode}</strong>
              </div>
              <div>
                <span>LED Status</span>
                <strong>{ledState}</strong>
              </div>
            </div>

            {/* Mode Control */}
            <div className="control-section">
              <h3>Operating Mode</h3>
              <div className="button-group">
                <button
                  className={`control-button ${
                    mode === "AUTO" ? "active" : ""
                  }`}
                  onClick={() => sendModeCommand("AUTO")}
                  disabled={controlLoading}
                >
                  AUTO
                </button>
                <button
                  className={`control-button ${
                    mode === "MANUAL" ? "active" : ""
                  }`}
                  onClick={() => sendModeCommand("MANUAL")}
                  disabled={controlLoading}
                >
                  MANUAL
                </button>
              </div>
            </div>

            {/* LED Control */}
            <div className="control-section">
              <h3>LED Control</h3>
              <div className="button-group">
                <button
                  className="control-button led-on"
                  onClick={() => sendLedCommand("ON")}
                  disabled={controlLoading || mode !== "MANUAL"}
                >
                  💡 LED ON
                </button>
                <button
                  className="control-button led-off"
                  onClick={() => sendLedCommand("OFF")}
                  disabled={controlLoading || mode !== "MANUAL"}
                >
                  LED OFF
                </button>
              </div>
              {mode === "AUTO" && (
                <p className="control-hint">
                  Switch to MANUAL mode to control the LED.
                </p>
              )}
            </div>

            {/* Control Message */}
            {controlMessage && (
              <div className="control-message">{controlMessage}</div>
            )}
          </div>
        </>
      )}
    </div>
  );
}

export default App;