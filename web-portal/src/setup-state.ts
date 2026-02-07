import { Api, WifiNetworkMetadata } from './api'
import { findAndAssertDomId } from './utils'

export class SetupState {
  readonly step1Container: HTMLDivElement
  readonly step1ContinueButton: HTMLButtonElement

  readonly step2Container: HTMLDivElement
  readonly step2WifiList: HTMLDivElement
  readonly step2PassphraseInput: HTMLInputElement
  readonly step2ContinueButton: HTMLButtonElement

  readonly step3Container: HTMLDivElement
  readonly step3ApiKeyInput: HTMLInputElement
  readonly step3ContinueButton: HTMLButtonElement

  readonly stepToContainerMap: (HTMLDivElement | undefined)[]

  // Valid values are 1, 2 and 3
  private setupStep: number
  private scanWifiIntervalJob: number | null = null
  private selectedWifiSsid: string | null = null

  private onWorkflowCompleteAction: (() => void) | undefined

  constructor() {
    this.step1Container = findAndAssertDomId('setup-screen-1')
    this.step2Container = findAndAssertDomId('setup-screen-2')
    this.step3Container = findAndAssertDomId('setup-screen-3')

    this.step1ContinueButton = findAndAssertDomId('setup-screen-1-continue-btn')
    this.step2WifiList = findAndAssertDomId('setup-screen-2-wifi-list')
    this.step2PassphraseInput = findAndAssertDomId(
      'setup-screen-2-passphrase-inp',
    )
    this.step2ContinueButton = findAndAssertDomId('setup-screen-2-continue-btn')
    this.step3ApiKeyInput = findAndAssertDomId('setup-screen-3-apikey-inp')
    this.step3ContinueButton = findAndAssertDomId('setup-screen-3-continue-btn')

    this.stepToContainerMap = [
      undefined,
      this.step1Container,
      this.step2Container,
      this.step3Container,
    ]

    this.bindStep1Actions()
    this.bindStep2Actions()
    this.bindStep3Actions()

    this.setupStep = 1
    this.displayWorkflowStep(1)
  }

  skipWifiSetup() {
    this.setupStep = 3
    this.displayWorkflowStep(3)
  }

  bindStep1Actions() {
    this.step1ContinueButton.addEventListener('click', () => {
      if (this.setupStep !== 1) {
        return
      }
      this.displayWorkflowStep(2)

      // Start scanning for WiFi networks
      this.scanWifiNetworks()
      this.scanWifiIntervalJob = setInterval(async () => {
        this.scanWifiNetworks()
      }, 10000)
    })
  }

  async scanWifiNetworks() {
    try {
      const { networks } = await Api.scanWifiNetworks()
      this.displayWifiNetworks(networks)
    } catch (error) {
      console.error('Failed to scan WiFi networks:', error)
    }
  }

  bindStep2Actions() {
    this.step2ContinueButton.addEventListener('click', async () => {
      if (this.setupStep !== 2) {
        return
      }
      if (!this.selectedWifiSsid) {
        alert('Please select a WiFi network')
        return
      }
      if (this.scanWifiIntervalJob !== null) {
        clearInterval(this.scanWifiIntervalJob)
        this.scanWifiIntervalJob = null
      }
      this.step2ContinueButton.disabled = true
      await Api.saveWifiNetwork(
        this.selectedWifiSsid,
        this.step2PassphraseInput.value,
      )
      this.step2ContinueButton.disabled = false
      this.displayWorkflowStep(3)
    })
  }

  bindStep3Actions() {
    this.step3ContinueButton.addEventListener('click', async () => {
      if (this.setupStep !== 3) {
        return
      }
      const apiKey = this.step3ApiKeyInput.value.trim()
      if (!apiKey) {
        alert('Please enter your API key')
        return
      }
      this.step3ContinueButton.disabled = true
      await Api.saveLtaApiKey(apiKey)
      this.step3ContinueButton.disabled = false
      this.onWorkflowCompleteAction?.()
    })
  }

  displayWorkflowStep(stepNumber: number) {
    this.setupStep = stepNumber

    // Update progress indicator
    this.updateProgressIndicator(stepNumber)

    // Show/hide step containers
    this.stepToContainerMap.forEach((container, idx) => {
      if (container === undefined) {
        return
      }

      if (idx === stepNumber) {
        container.classList.remove('hidden')
        // Trigger animation by removing and re-adding fade-in class
        const card = container.querySelector('.setup-card')
        if (card) {
          card.classList.remove('fade-in')
          card.classList.add('fade-in')
        }
      } else {
        container.classList.add('hidden')
      }
    })
  }

  updateProgressIndicator(currentStep: number) {
    const progressSteps = document.querySelectorAll('.progress-step')
    const progressLines = document.querySelectorAll('.progress-line')

    progressSteps.forEach((step, index) => {
      const stepNum = index + 1
      if (stepNum < currentStep) {
        step.classList.add('completed')
        step.classList.remove('active')
      } else if (stepNum === currentStep) {
        step.classList.add('active')
        step.classList.remove('completed')
      } else {
        step.classList.remove('active', 'completed')
      }
    })

    progressLines.forEach((line, index) => {
      if (index + 1 < currentStep) {
        line.classList.add('completed')
      } else {
        line.classList.remove('completed')
      }
    })
  }

  displayWifiNetworks(networks: WifiNetworkMetadata[]) {
    // Clear existing networks
    while (this.step2WifiList.lastChild) {
      this.step2WifiList.removeChild(this.step2WifiList.lastChild)
    }

    if (networks.length === 0) {
      const emptyMessage = document.createElement('div')
      emptyMessage.className = 'scanning-indicator'
      const dotAnimation = document.createElement('div')
      dotAnimation.className = 'pulse-dot'
      emptyMessage.appendChild(dotAnimation)
      const helperText = document.createElement('div')
      helperText.textContent = 'Scanning for networks...'
      emptyMessage.appendChild(helperText)
      this.step2WifiList.appendChild(emptyMessage)
      return
    }

    networks.forEach((network) => {
      const row = document.createElement('div')
      row.className = 'list-item'
      row.textContent = network.ssid

      if (this.selectedWifiSsid === network.ssid) {
        row.classList.add('selected')
      }

      row.addEventListener('click', () => {
        // Remove selected class from all items
        this.step2WifiList.querySelectorAll('.list-item').forEach((item) => {
          item.classList.remove('selected')
        })
        // Add selected class to clicked item
        row.classList.add('selected')
        this.selectedWifiSsid = network.ssid
      })

      this.step2WifiList.appendChild(row)
    })
  }

  onWorkflowComplete(action: () => void) {
    this.onWorkflowCompleteAction = action
  }
}
