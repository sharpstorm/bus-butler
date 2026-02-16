import express from 'express'
import { MOCK_SCENARIOS } from './mock-scenarios.js'

// Mock data storage - initialized from scenarios
const mockState = {
  wifi: {
    ssid: MOCK_SCENARIOS.initialWifiSsid,
    passphrase: null,
  },
  lta: {
    apiKey: MOCK_SCENARIOS.initialLtaApiKey,
    busStopCode: MOCK_SCENARIOS.initialBusStopCode,
    busServiceOrder: MOCK_SCENARIOS.initialBusServiceOrder || '',
  },
  weather: {
    town: MOCK_SCENARIOS.initialWeatherTown,
    region: MOCK_SCENARIOS.initialWeatherRegion,
  },
}

// Helper to add artificial delay
const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms))

// Helper to handle mock delays and errors
async function handleRequest(res, handler) {
  if (MOCK_SCENARIOS.simulateSlowNetwork) {
    await delay(MOCK_SCENARIOS.networkDelay)
  }

  if (MOCK_SCENARIOS.simulateErrors && Math.random() > 0.7) {
    return res.status(500).json({ error: 'Random simulated error' })
  }

  return handler()
}

export function createMockApiMiddleware() {
  // Create Express app for API routes
  const app = express()

  // Enable JSON parsing
  app.use(express.json())

  // WiFi APIs
  app.get('/api/wifi/scan', async (req, res) => {
    await handleRequest(res, () => {
      res.json({
        networks: [
          { ssid: 'Home WiFi', rssi: -45, security: 'WPA2' },
          { ssid: 'Office Network', rssi: -62, security: 'WPA2+WPA3' },
          { ssid: 'Guest WiFi', rssi: -75, security: 'WPA2' },
          { ssid: 'Neighbor WiFi', rssi: -80, security: 'WPA2' },
          { ssid: 'Public WiFi', rssi: -85, security: 'open' },
        ],
      })
    })
  })

  app.post('/api/wifi/config', async (req, res) => {
    await handleRequest(res, () => {
      const { ssid, passphrase } = req.body
      if (!ssid || !passphrase) {
        return res.status(400).json({ success: false })
      }
      mockState.wifi.ssid = ssid
      mockState.wifi.passphrase = passphrase
      console.log(`✓ WiFi configured: ${ssid}`)
      res.json({ success: true })
    })
  })

  app.get('/api/wifi/config', async (req, res) => {
    await handleRequest(res, () => {
      res.json({ ssid: mockState.wifi.ssid })
    })
  })

  // LTA DataMall APIs
  app.post('/api/config/lta-api', async (req, res) => {
    await handleRequest(res, () => {
      const { apiKey } = req.body
      if (!apiKey) {
        return res.status(400).json({ success: false })
      }
      mockState.lta.apiKey = apiKey
      console.log(`✓ LTA API key saved: ${apiKey.substring(0, 10)}...`)
      res.json({ success: true })
    })
  })

  app.get('/api/config/lta-api', async (req, res) => {
    await handleRequest(res, () => {
      res.json({ isApiKeySaved: !!mockState.lta.apiKey })
    })
  })

  app.post('/api/config/busstop-code', async (req, res) => {
    await handleRequest(res, () => {
      const { busStopCode } = req.body
      if (!busStopCode) {
        return res.status(400).json({ success: false })
      }
      mockState.lta.busStopCode = busStopCode
      console.log(`✓ Bus stop code saved: ${busStopCode}`)
      res.json({ success: true })
    })
  })

  app.get('/api/config/busstop-code', async (req, res) => {
    await handleRequest(res, () => {
      res.json({ busStopCode: mockState.lta.busStopCode })
    })
  })

  // Bus Service Order APIs
  app.get('/api/config/bus-service-order', async (req, res) => {
    await handleRequest(res, () => {
      res.json({ order: mockState.lta.busServiceOrder })
    })
  })

  app.post('/api/config/bus-service-order', async (req, res) => {
    await handleRequest(res, () => {
      const { order } = req.body
      if (order === undefined) {
        return res.status(400).json({ success: false })
      }
      mockState.lta.busServiceOrder = order
      console.log(`✓ Bus service order saved: ${order}`)
      res.json({ success: true })
    })
  })

  // Weather APIs
  app.post('/api/config/weather-town', async (req, res) => {
    await handleRequest(res, () => {
      const { town } = req.body
      if (!town) {
        return res.status(400).json({ success: false })
      }
      mockState.weather.town = town.trim()
      console.log(`✓ Weather town saved: ${town}`)
      res.json({ success: true })
    })
  })

  app.get('/api/config/weather-town', async (req, res) => {
    await handleRequest(res, () => {
      res.json({ town: mockState.weather.town })
    })
  })

  app.post('/api/config/weather-region', async (req, res) => {
    await handleRequest(res, () => {
      const { region } = req.body
      if (!region) {
        return res.status(400).json({ success: false })
      }
      mockState.weather.region = region
      console.log(`✓ Weather region saved: ${region}`)
      res.json({ success: true })
    })
  })

  app.get('/api/config/weather-region', async (req, res) => {
    await handleRequest(res, () => {
      res.json({ region: mockState.weather.region })
    })
  })

  app.get('/api/config/weather-region-list', async (req, res) => {
    await handleRequest(res, () => {
      res.json({
        region: ['west', 'east', 'central', 'north', 'south'],
      })
    })
  })

  app.get('/api/config/weather-town-list', async (req, res) => {
    await handleRequest(res, () => {
      res.json({
        towns: [
          'Ang Mo Kio',
          'Bedok',
          'Bishan',
          'Boon Lay',
          'Bukit Batok',
          'Bukit Merah',
          'Bukit Panjang',
          'Bukit Timah',
          'Central Water Catchment',
          'Changi',
          'Choa Chu Kang',
          'Clementi',
          'City',
          'Geylang',
          'Hougang',
          'Jalan Bahar',
          'Jurong East',
          'Jurong Island',
          'Jurong West',
          'Kallang',
          'Lim Chu Kang',
          'Mandai',
          'Marine Parade',
          'Novena',
          'Pasir Ris',
          'Paya Lebar',
          'Pioneer',
          'Punggol',
          'Queenstown',
          'Sembawang',
          'Sengkang',
          'Serangoon',
          'Sentosa',
          'Seletar',
          'Singapore Botanic Gardens',
          'Tampines',
          'Tanglin',
          'Tengah',
          'Toa Payoh',
          'Tuas',
          'Western Islands',
          'Western Water Catchment',
          'Woodlands',
          'Yishun',
        ],
      })
    })
  })

  // Hardware Config
  app.post('/api/config/screen-brightness', async (req, res) => {
    await handleRequest(res, () => {
      const { brightness } = req.body
      console.log(`✓ Screen brightness set: ${brightness}`)
      res.json({ success: true })
    })
  })

  app.get('/api/config/screen-brightness', async (req, res) => {
    await handleRequest(res, () => {
      res.json({ brightness: 128 })
    })
  })

  // Test APIs
  app.post('/api/test/bus-arrival', async (req, res) => {
    await handleRequest(res, () => {
      const { busStopCode } = req.body

      if (!busStopCode) {
        return res.status(400).json({ error: 'No busStopCode' })
      }

      if (MOCK_SCENARIOS.emptyBusData) {
        return res.json({
          busStop: busStopCode,
          timestamp: Math.floor(Date.now() / 1000),
          services: [],
        })
      }

      console.log(`✓ Fetching bus arrivals for stop: ${busStopCode}`)

      const now = Math.floor(Date.now() / 1000)

      const services = []
      const serviceCount = Math.min(MOCK_SCENARIOS.busServiceCount, 12)

      const mockServices = [
        { number: '73', company: 'SBST' },
        { number: '156', company: 'SMRT' },
        { number: '74', company: 'SBST' },
        { number: '88', company: 'SMRT' },
        { number: '130', company: 'SBST' },
        { number: '188', company: 'SMRT' },
        { number: '190', company: 'SBST' },
        { number: '65', company: 'SMRT' },
        { number: '80', company: 'SBST' },
        { number: '961', company: 'SMRT' },
        { number: '963', company: 'SBST' },
        { number: '188R', company: 'SMRT' },
      ]

      for (let i = 0; i < serviceCount; i++) {
        const service = mockServices[i]
        const firstBusTime =
          MOCK_SCENARIOS.hasArrivingBus && i === 0 ? 0 : 60 + i * 60

        services.push({
          service: service.number,
          company: service.company,
          nextBus1: {
            arrival: now + firstBusTime,
            secondsLeft: firstBusTime,
            busType: i % 2 === 0 ? 'Single Deck' : 'Double Deck',
            busLoad: [
              'Seats Available',
              'Standing Available',
              'Limited Standing',
            ][i % 3],
            isWAB: i % 2 === 0,
          },
          nextBus2: {
            arrival: now + firstBusTime + 360,
            secondsLeft: firstBusTime + 360,
            busType: i % 2 === 1 ? 'Single Deck' : 'Double Deck',
            busLoad: ['Seats Available', 'Standing Available'][i % 2],
            isWAB: i % 2 === 1,
          },
          nextBus3: {
            arrival: now + firstBusTime + 720,
            secondsLeft: firstBusTime + 720,
            busType: 'Single Deck',
            busLoad: 'Seats Available',
            isWAB: true,
          },
        })
      }

      res.json({
        busStop: busStopCode,
        timestamp: now,
        services,
      })
    })
  })

  app.post('/api/test/weather', async (req, res) => {
    await handleRequest(res, () => {
      if (MOCK_SCENARIOS.weatherApiFailed) {
        return res.status(502).json({ error: 'NEA API request failed' })
      }

      if (!mockState.weather.town || !mockState.weather.region) {
        return res.status(422).json({
          error: 'Weather town or region not configured',
        })
      }

      console.log(
        `✓ Fetching weather for: ${mockState.weather.town}, ${mockState.weather.region}`,
      )

      res.json({
        town: mockState.weather.town,
        region: mockState.weather.region,
        twoHourForecast: {
          forecast: MOCK_SCENARIOS.weatherForecast.twoHour,
        },
        twentyFourHourForecast: {
          periods: [
            {
              timePeriod: 'Morning',
              forecast: MOCK_SCENARIOS.weatherForecast.morning,
            },
            {
              timePeriod: 'Afternoon',
              forecast: MOCK_SCENARIOS.weatherForecast.afternoon,
            },
            {
              timePeriod: 'Night',
              forecast: MOCK_SCENARIOS.weatherForecast.night,
            },
          ],
        },
      })
    })
  })

  console.log('\n🚀 Mock API Server Ready')
  console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━')
  console.log('📍 All /api/* endpoints are mocked')
  console.log('\n📝 Current Mock Scenarios:')
  console.log(`   Initial WiFi: ${mockState.wifi.ssid || 'Not configured'}`)
  console.log(`   Initial Bus Stop: ${mockState.lta.busStopCode}`)
  console.log(`   Initial Bus Service Order: ${mockState.lta.busServiceOrder || 'None (uses API order)'}`)
  console.log(
    `   Initial Weather: ${mockState.weather.town}, ${mockState.weather.region}`,
  )
  console.log(`   Simulate Errors: ${MOCK_SCENARIOS.simulateErrors}`)
  console.log(`   Slow Network: ${MOCK_SCENARIOS.simulateSlowNetwork}`)
  if (MOCK_SCENARIOS.simulateSlowNetwork) {
    console.log(`   Network Delay: ${MOCK_SCENARIOS.networkDelay}ms`)
  }
  console.log(`   Bus Service Count: ${MOCK_SCENARIOS.busServiceCount}`)
  console.log(`   Empty Bus Data: ${MOCK_SCENARIOS.emptyBusData}`)
  console.log(`   Weather API Failed: ${MOCK_SCENARIOS.weatherApiFailed}`)
  console.log('\n💡 Edit mock-scenarios.js to change test scenarios')
  console.log('━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n')

  // Return the Express app as middleware
  return app
}
