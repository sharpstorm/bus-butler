import { Api, BusArrivalRespBody, WeatherRespBody, BusService } from './api'

export class DashboardState {
  constructor() {
    this.initTabs()
    this.initBusArrival()
    this.initWeather()
  }

  private initTabs() {
    const tabs = document.querySelectorAll('.tab-btn')
    tabs.forEach((tab) => {
      tab.addEventListener('click', (e) => {
        const target = e.currentTarget as HTMLElement
        const tabName = target.dataset.tab as 'settings' | 'bus' | 'weather'
        this.switchTab(tabName)
      })
    })
  }

  private switchTab(tabName: 'settings' | 'bus' | 'weather') {
    // Update tab buttons
    document.querySelectorAll('.tab-btn').forEach((btn) => {
      btn.classList.toggle('active', btn.getAttribute('data-tab') === tabName)
    })

    // Update tab content
    document.querySelectorAll('.tab-content').forEach((content) => {
      content.classList.toggle(
        'active',
        content.getAttribute('data-tab') === tabName,
      )
    })
  }

  // Settings Tab
  async initSettings() {
    // Load weather regions
    await this.loadWeatherRegions()

    // Load current settings
    await this.loadCurrentSettings()

    // WiFi reconfiguration
    const wifiBtn = document.getElementById('reconfigure-wifi-btn')
    wifiBtn?.addEventListener('click', () => this.reconfigureWifi())

    // Bus stop configuration
    const busStopForm = document.getElementById(
      'bus-stop-form',
    ) as HTMLFormElement
    busStopForm?.addEventListener('submit', (e) => {
      e.preventDefault()
      this.saveBusStop()
    })

    // Weather town configuration
    const weatherTownForm = document.getElementById(
      'weather-town-form',
    ) as HTMLFormElement
    weatherTownForm?.addEventListener('submit', (e) => {
      e.preventDefault()
      this.saveWeatherTown()
    })

    // Weather region configuration
    const weatherRegionForm = document.getElementById(
      'weather-region-form',
    ) as HTMLFormElement
    weatherRegionForm?.addEventListener('submit', (e) => {
      e.preventDefault()
      this.saveWeatherRegion()
    })

    // LTA API key configuration
    const apiKeyForm = document.getElementById(
      'lta-api-form',
    ) as HTMLFormElement
    apiKeyForm?.addEventListener('submit', (e) => {
      e.preventDefault()
      this.saveLtaApiKey()
    })

    // Load weather towns
    const loadTownsBtn = document.getElementById('load-weather-towns-btn')
    loadTownsBtn?.addEventListener('click', () => this.loadWeatherTowns())
  }

  private async loadCurrentSettings() {
    try {
      // Load WiFi
      const wifiResp = await Api.getConfiguredWifiSsid()
      const wifiStatus = document.getElementById('current-wifi-status')
      if (wifiStatus) {
        wifiStatus.textContent = wifiResp.ssid || 'Not configured'
      }

      // Load bus stop
      const busStopResp = await Api.getBusStopCode()
      const busStopInput = document.getElementById(
        'bus-stop-input',
      ) as HTMLInputElement
      if (busStopInput && busStopResp.busStopCode) {
        busStopInput.value = busStopResp.busStopCode
      }

      // Load weather town
      const townResp = await Api.getWeatherTown()
      const townInput = document.getElementById(
        'weather-town-input',
      ) as HTMLInputElement
      if (townInput && townResp.town) {
        townInput.value = townResp.town
      }

      // Load weather region
      const regionResp = await Api.getWeatherRegion()
      const regionSelect = document.getElementById(
        'weather-region-select',
      ) as HTMLSelectElement
      if (regionSelect && regionResp.region) {
        regionSelect.value = regionResp.region
      }
    } catch (error) {
      console.error('Error loading settings:', error)
    }
  }

  private async loadWeatherRegions() {
    try {
      const resp = await Api.getWeatherRegionList()
      const select = document.getElementById(
        'weather-region-select',
      ) as HTMLSelectElement
      if (select) {
        // Clear existing options
        while (select.firstChild) {
          select.removeChild(select.firstChild)
        }

        // Add default option
        const defaultOption = document.createElement('option')
        defaultOption.value = ''
        defaultOption.textContent = 'Select region...'
        select.appendChild(defaultOption)

        // Add region options
        resp.region.forEach((region) => {
          const option = document.createElement('option')
          option.value = region
          option.textContent = region.charAt(0).toUpperCase() + region.slice(1)
          select.appendChild(option)
        })
      }
    } catch (error) {
      console.error('Error loading regions:', error)
    }
  }

  private async loadWeatherTowns() {
    const btn = document.getElementById(
      'load-weather-towns-btn',
    ) as HTMLButtonElement
    const townsList = document.getElementById('weather-towns-list')

    if (!btn || !townsList) return

    btn.disabled = true
    btn.textContent = 'Loading...'

    // Clear existing content
    while (townsList.firstChild) {
      townsList.removeChild(townsList.firstChild)
    }

    const loadingDiv = document.createElement('div')
    loadingDiv.className = 'loading-text'
    loadingDiv.textContent = 'Loading available towns...'
    townsList.appendChild(loadingDiv)

    try {
      const resp = await Api.getWeatherTownList()

      // Clear loading message
      while (townsList.firstChild) {
        townsList.removeChild(townsList.firstChild)
      }

      if (resp.towns.length === 0) {
        const noData = document.createElement('p')
        noData.className = 'no-data'
        noData.textContent = 'No towns available'
        townsList.appendChild(noData)
      } else {
        const grid = document.createElement('div')
        grid.className = 'towns-grid'

        resp.towns.forEach((town) => {
          const item = document.createElement('div')
          item.className = 'town-item'
          item.textContent = town
          grid.appendChild(item)
        })

        townsList.appendChild(grid)
      }
    } catch (error) {
      // Clear loading message
      while (townsList.firstChild) {
        townsList.removeChild(townsList.firstChild)
      }

      const errorText = document.createElement('p')
      errorText.className = 'error-text'
      errorText.textContent = 'Error loading towns'
      townsList.appendChild(errorText)
    } finally {
      btn.disabled = false
      btn.textContent = 'Load Available Towns'
    }
  }

  private reconfigureWifi() {
    if (
      confirm('This will restart the device into WiFi setup mode. Continue?')
    ) {
      window.location.reload()
    }
  }

  private async saveBusStop() {
    const input = document.getElementById('bus-stop-input') as HTMLInputElement
    const status = document.getElementById('bus-stop-status')

    if (!input || !status) return

    const busStopCode = input.value.trim()
    if (!busStopCode) {
      this.showStatus(status, 'Please enter a bus stop code', 'error')
      return
    }

    try {
      await Api.saveBusStopCode(busStopCode)
      this.showStatus(status, 'Bus stop code saved!', 'success')
    } catch (error) {
      this.showStatus(status, 'Error saving bus stop code', 'error')
    }
  }

  private async saveWeatherTown() {
    const input = document.getElementById(
      'weather-town-input',
    ) as HTMLInputElement
    const status = document.getElementById('weather-town-status')

    if (!input || !status) return

    const town = input.value.trim()
    if (!town) {
      this.showStatus(status, 'Please enter a town name', 'error')
      return
    }

    try {
      await Api.saveWeatherTown(town)
      this.showStatus(status, 'Weather town saved!', 'success')
    } catch (error) {
      this.showStatus(status, 'Error saving weather town', 'error')
    }
  }

  private async saveWeatherRegion() {
    const select = document.getElementById(
      'weather-region-select',
    ) as HTMLSelectElement
    const status = document.getElementById('weather-region-status')

    if (!select || !status) return

    const region = select.value
    if (!region) {
      this.showStatus(status, 'Please select a region', 'error')
      return
    }

    try {
      await Api.saveWeatherRegion(region)
      this.showStatus(status, 'Weather region saved!', 'success')
    } catch (error) {
      this.showStatus(status, 'Error saving weather region', 'error')
    }
  }

  private async saveLtaApiKey() {
    const input = document.getElementById('lta-api-input') as HTMLInputElement
    const status = document.getElementById('lta-api-status')

    if (!input || !status) return

    const apiKey = input.value.trim()
    if (!apiKey) {
      this.showStatus(status, 'Please enter an API key', 'error')
      return
    }

    try {
      await Api.saveLtaApiKey(apiKey)
      this.showStatus(status, 'API key saved!', 'success')
      input.value = ''
    } catch (error) {
      this.showStatus(status, 'Error saving API key', 'error')
    }
  }

  // Bus Arrival Tab
  private initBusArrival() {
    const fetchBtn = document.getElementById('fetch-bus-btn')
    fetchBtn?.addEventListener('click', () => this.fetchBusArrivals())
  }

  private async fetchBusArrivals() {
    const btn = document.getElementById('fetch-bus-btn') as HTMLButtonElement
    const results = document.getElementById('bus-results')

    if (!btn || !results) return

    btn.disabled = true
    btn.textContent = 'Fetching...'

    // Clear existing content
    while (results.firstChild) {
      results.removeChild(results.firstChild)
    }

    const loadingDiv = document.createElement('div')
    loadingDiv.className = 'loading-text'
    loadingDiv.textContent = 'Loading bus arrival data...'
    results.appendChild(loadingDiv)

    try {
      const busStopResp = await Api.getBusStopCode()
      if (!busStopResp.busStopCode) {
        throw new Error('No bus stop configured')
      }

      const data = await Api.testBusArrival(busStopResp.busStopCode)

      // Clear loading message
      while (results.firstChild) {
        results.removeChild(results.firstChild)
      }

      this.displayBusArrivals(results, data)
    } catch (error: any) {
      // Clear loading message
      while (results.firstChild) {
        results.removeChild(results.firstChild)
      }

      this.displayError(results, error.message || 'Error fetching bus arrivals')
    } finally {
      btn.disabled = false
      btn.textContent = 'Fetch Bus Arrivals'
    }
  }

  private displayBusArrivals(container: HTMLElement, data: BusArrivalRespBody) {
    if (data.services.length === 0) {
      const noData = document.createElement('p')
      noData.className = 'no-data'
      noData.textContent = 'No buses currently arriving at this stop'
      container.appendChild(noData)
      return
    }

    // Header
    const header = document.createElement('div')
    header.className = 'bus-results-header'
    const headerTitle = document.createElement('h3')
    headerTitle.textContent = `Bus Stop: ${data.busStop}`
    header.appendChild(headerTitle)
    container.appendChild(header)

    // Services
    data.services.forEach((service) => {
      const card = this.createBusServiceCard(service)
      container.appendChild(card)
    })
  }

  private createBusServiceCard(service: BusService): HTMLElement {
    const card = document.createElement('div')
    card.className = 'bus-service-card'

    // Header
    const header = document.createElement('div')
    header.className = 'bus-service-header'

    const busNumber = document.createElement('div')
    busNumber.className = 'bus-number'
    busNumber.textContent = service.service
    header.appendChild(busNumber)

    const busCompany = document.createElement('div')
    busCompany.className = 'bus-company'
    busCompany.textContent = service.company
    header.appendChild(busCompany)

    card.appendChild(header)

    // Arrivals
    const arrivals = document.createElement('div')
    arrivals.className = 'bus-arrivals'

    if (service.nextBus1) {
      arrivals.appendChild(this.createBusArrivalItem('Next', service.nextBus1))
    }
    if (service.nextBus2) {
      arrivals.appendChild(
        this.createBusArrivalItem('Following', service.nextBus2),
      )
    }
    if (service.nextBus3) {
      arrivals.appendChild(
        this.createBusArrivalItem('Subsequent', service.nextBus3),
      )
    }

    card.appendChild(arrivals)
    return card
  }

  private createBusArrivalItem(label: string, arrival: any): HTMLElement {
    const item = document.createElement('div')
    item.className = 'bus-arrival-item'

    // Label
    const arrivalLabel = document.createElement('div')
    arrivalLabel.className = 'arrival-label'
    arrivalLabel.textContent = label
    item.appendChild(arrivalLabel)

    // Time
    const minutes = Math.floor(arrival.secondsLeft / 60)
    const timeText = minutes <= 0 ? 'Arriving' : `${minutes} min`

    const arrivalTime = document.createElement('div')
    arrivalTime.className = 'arrival-time'
    arrivalTime.textContent = timeText
    item.appendChild(arrivalTime)

    // Details
    const details = document.createElement('div')
    details.className = 'arrival-details'

    const busType = document.createElement('span')
    busType.className = 'bus-type'
    busType.textContent = arrival.busType
    details.appendChild(busType)

    const busLoad = document.createElement('span')
    busLoad.className = `bus-load load-${arrival.busLoad
      .toLowerCase()
      .replace(/\s+/g, '-')}`
    busLoad.textContent = arrival.busLoad
    details.appendChild(busLoad)

    if (arrival.isWAB) {
      const wabBadge = document.createElement('span')
      wabBadge.className = 'wab-badge'
      wabBadge.textContent = 'WAB'
      details.appendChild(wabBadge)
    }

    item.appendChild(details)
    return item
  }

  // Weather Tab
  private initWeather() {
    const fetchBtn = document.getElementById('fetch-weather-btn')
    fetchBtn?.addEventListener('click', () => this.fetchWeather())
  }

  private async fetchWeather() {
    const btn = document.getElementById(
      'fetch-weather-btn',
    ) as HTMLButtonElement
    const results = document.getElementById('weather-results')

    if (!btn || !results) return

    btn.disabled = true
    btn.textContent = 'Fetching...'

    // Clear existing content
    while (results.firstChild) {
      results.removeChild(results.firstChild)
    }

    const loadingDiv = document.createElement('div')
    loadingDiv.className = 'loading-text'
    loadingDiv.textContent = 'Loading weather data...'
    results.appendChild(loadingDiv)

    try {
      const data = await Api.testWeather()

      // Clear loading message
      while (results.firstChild) {
        results.removeChild(results.firstChild)
      }

      this.displayWeather(results, data)
    } catch (error: any) {
      // Clear loading message
      while (results.firstChild) {
        results.removeChild(results.firstChild)
      }

      this.displayError(results, error.message || 'Error fetching weather data')
    } finally {
      btn.disabled = false
      btn.textContent = 'Fetch Weather'
    }
  }

  private displayWeather(container: HTMLElement, data: WeatherRespBody) {
    const card = document.createElement('div')
    card.className = 'weather-card'

    // Header
    const header = document.createElement('div')
    header.className = 'weather-header'

    const title = document.createElement('h3')
    title.textContent = data.town
    header.appendChild(title)

    const regionBadge = document.createElement('span')
    regionBadge.className = 'region-badge'
    regionBadge.textContent = data.region
    header.appendChild(regionBadge)

    card.appendChild(header)

    // 2-hour forecast section
    const twoHourSection = document.createElement('div')
    twoHourSection.className = 'weather-section'

    const twoHourTitle = document.createElement('h4')
    twoHourTitle.textContent = '2-Hour Forecast'
    twoHourSection.appendChild(twoHourTitle)

    const forecastItem = document.createElement('div')
    forecastItem.className = 'forecast-item'

    const forecastIcon = document.createElement('div')
    forecastIcon.className = 'forecast-icon'
    forecastIcon.textContent = this.getWeatherIcon(
      data.twoHourForecast.forecast,
    )
    forecastItem.appendChild(forecastIcon)

    const forecastText = document.createElement('div')
    forecastText.className = 'forecast-text'
    forecastText.textContent = data.twoHourForecast.forecast
    forecastItem.appendChild(forecastText)

    twoHourSection.appendChild(forecastItem)
    card.appendChild(twoHourSection)

    // 24-hour forecast section
    const twentyFourSection = document.createElement('div')
    twentyFourSection.className = 'weather-section'

    const twentyFourTitle = document.createElement('h4')
    twentyFourTitle.textContent = '24-Hour Forecast'
    twentyFourSection.appendChild(twentyFourTitle)

    const periods = document.createElement('div')
    periods.className = 'forecast-periods'

    data.twentyFourHourForecast.periods.forEach((period) => {
      const periodItem = document.createElement('div')
      periodItem.className = 'period-item'

      const periodTime = document.createElement('div')
      periodTime.className = 'period-time'
      periodTime.textContent = period.timePeriod
      periodItem.appendChild(periodTime)

      const periodForecast = document.createElement('div')
      periodForecast.className = 'period-forecast'

      const periodIcon = document.createElement('span')
      periodIcon.className = 'forecast-icon-small'
      periodIcon.textContent = this.getWeatherIcon(period.forecast)
      periodForecast.appendChild(periodIcon)

      periodForecast.appendChild(document.createTextNode(period.forecast))
      periodItem.appendChild(periodForecast)

      periods.appendChild(periodItem)
    })

    twentyFourSection.appendChild(periods)
    card.appendChild(twentyFourSection)

    container.appendChild(card)
  }

  private displayError(container: HTMLElement, message: string) {
    const errorCard = document.createElement('div')
    errorCard.className = 'error-card'

    // SVG Icon
    const svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg')
    svg.setAttribute('width', '24')
    svg.setAttribute('height', '24')
    svg.setAttribute('viewBox', '0 0 24 24')
    svg.setAttribute('fill', 'none')
    svg.setAttribute('stroke', 'currentColor')
    svg.setAttribute('stroke-width', '2')

    const circle = document.createElementNS(
      'http://www.w3.org/2000/svg',
      'circle',
    )
    circle.setAttribute('cx', '12')
    circle.setAttribute('cy', '12')
    circle.setAttribute('r', '10')
    svg.appendChild(circle)

    const line1 = document.createElementNS('http://www.w3.org/2000/svg', 'line')
    line1.setAttribute('x1', '12')
    line1.setAttribute('y1', '8')
    line1.setAttribute('x2', '12')
    line1.setAttribute('y2', '12')
    svg.appendChild(line1)

    const line2 = document.createElementNS('http://www.w3.org/2000/svg', 'line')
    line2.setAttribute('x1', '12')
    line2.setAttribute('y1', '16')
    line2.setAttribute('x2', '12.01')
    line2.setAttribute('y2', '16')
    svg.appendChild(line2)

    errorCard.appendChild(svg)

    const errorMessage = document.createElement('p')
    errorMessage.textContent = message
    errorCard.appendChild(errorMessage)

    container.appendChild(errorCard)
  }

  private getWeatherIcon(forecast: string): string {
    const lower = forecast.toLowerCase()
    if (lower.includes('thunder') || lower.includes('storm')) return '⛈️'
    if (lower.includes('rain') || lower.includes('shower')) return '🌧️'
    if (lower.includes('cloudy')) return '☁️'
    if (lower.includes('partly')) return '⛅'
    if (lower.includes('fair') || lower.includes('sunny')) return '☀️'
    return '🌤️'
  }

  private showStatus(
    element: HTMLElement,
    message: string,
    type: 'success' | 'error',
  ) {
    element.textContent = message
    element.className = `status-message status-${type}`
    setTimeout(() => {
      element.textContent = ''
      element.className = 'status-message'
    }, 3000)
  }
}
