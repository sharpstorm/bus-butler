/**
 * Mock API Scenarios Configuration
 * 
 * Edit these values to test different states and behaviors
 * Changes will apply on next page refresh
 */

export const MOCK_SCENARIOS = {
  // ==========================================
  // NETWORK SIMULATION
  // ==========================================

  /**
   * Simulate random errors (30% chance of error)
   * Useful for testing error handling
   */
  simulateErrors: false,

  /**
   * Simulate slow network connections
   * Useful for testing loading states
   */
  simulateSlowNetwork: false,

  /**
   * Network delay in milliseconds (when simulateSlowNetwork is true)
   */
  networkDelay: 2000,

  // ==========================================
  // INITIAL STATE
  // ==========================================

  /**
   * Set initial WiFi state
   * null = not configured (shows setup screen)
   * string = configured SSID (shows dashboard)
   */
  initialWifiSsid: null, // Try: 'Home WiFi' to skip setup

  /**
   * Set initial LTA API key state
   */
  initialLtaApiKey: null, // Try: 'test-api-key-123'

  /**
   * Set initial bus stop code
   */
  initialBusStopCode: '83139',

  /**
   * Set initial bus service order (comma-separated)
   * Empty string = no custom order (uses API order)
   * Example: '73,156,74' to show services in that order
   */
  initialBusServiceOrder: ['73', '156', '74', '88', '130'],

  /**
   * Set initial weather configuration
   */
  initialWeatherTown: 'Ang Mo Kio',
  initialWeatherRegion: 'central',

  // ==========================================
  // BUS ARRIVAL SCENARIOS
  // ==========================================

  /**
   * Return empty bus data (no services)
   * Useful for testing "no data" state
   */
  emptyBusData: false,

  /**
   * Number of bus services to return (1-12)
   * Set to 7+ to test pagination (>6 services)
   */
  busServiceCount: 8,

  /**
   * Simulate arriving bus (0 seconds)
   */
  hasArrivingBus: false,

  // ==========================================
  // WEATHER SCENARIOS
  // ==========================================

  /**
   * Simulate weather API failure
   * Useful for testing error handling
   */
  weatherApiFailed: false,

  /**
   * Weather forecast to return
   */
  weatherForecast: {
    twoHour: 'Partly Cloudy',
    morning: 'Partly Cloudy',
    afternoon: 'Thundery Showers',
    night: 'Fair',
    midnight: 'Fair',
  },
}

// ==========================================
// PRESET SCENARIOS
// ==========================================

/**
 * Apply a preset scenario for quick testing
 * Uncomment one to use it:
 */

// Fresh setup - no configuration
// Object.assign(MOCK_SCENARIOS, {
//   initialWifiSsid: null,
//   initialLtaApiKey: null,
// })

// Fully configured device
// Object.assign(MOCK_SCENARIOS, {
//   initialWifiSsid: 'Home WiFi',
//   initialLtaApiKey: 'test-api-key',
//   initialBusStopCode: '83139',
//   initialWeatherTown: 'Ang Mo Kio',
//   initialWeatherRegion: 'central',
// })

// Slow network testing
// Object.assign(MOCK_SCENARIOS, {
//   simulateSlowNetwork: true,
//   networkDelay: 3000,
// })

// Error handling testing
// Object.assign(MOCK_SCENARIOS, {
//   simulateErrors: true,
//   emptyBusData: true,
//   weatherApiFailed: true,
// })
