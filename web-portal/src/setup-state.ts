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

  bindStep1Actions() {
    this.step1ContinueButton.addEventListener('click', () => {
      if (this.setupStep !== 1) {
        return
      }
      this.displayWorkflowStep(2)
      this.scanWifiIntervalJob = setInterval(async () => {
        const { networks } = await Api.scanWifiNetworks()
        this.displayWifiNetworks(networks)
      }, 10000)
    })
  }

  bindStep2Actions() {
    this.step2ContinueButton.addEventListener('click', () => {
      if (this.setupStep !== 2) {
        return
      }
      if (this.scanWifiIntervalJob !== null) {
        clearInterval(this.scanWifiIntervalJob)
        this.scanWifiIntervalJob = null
      }
      this.displayWorkflowStep(3)
    })
  }

  bindStep3Actions() {
    this.step3ContinueButton.addEventListener('click', () => {
      if (this.setupStep !== 3) {
        return
      }
      this.onWorkflowCompleteAction?.()
    })
  }

  displayWorkflowStep(stepNumber: number) {
    this.setupStep = stepNumber
    this.stepToContainerMap.forEach((container, idx) => {
      if (container === undefined) {
        return
      }

      if (idx === stepNumber) {
        container.classList.remove('hidden')
      } else {
        container.classList.add('hidden')
      }
    })
  }

  displayWifiNetworks(networks: WifiNetworkMetadata[]) {
    while (this.step2WifiList.lastChild) {
      this.step2WifiList.removeChild(this.step2WifiList.lastChild)
    }

    networks.forEach((network) => {
      const row = document.createElement('a')
      row.classList.add('list-row')
      row.textContent = network.ssid
      this.step2WifiList.appendChild(row)
    })
  }

  onWorkflowComplete(action: () => void) {
    this.onWorkflowCompleteAction = action
  }
}
