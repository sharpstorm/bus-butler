export type WifiNetworkMetadata = {
  ssid: string
  rssi: number
  security: string
}

export type ScanWifiNetworksRespBody = {
  networks: WifiNetworkMetadata[]
}

export class Api {
  static async scanWifiNetworks() {
    const resp = await fetch('/api/wifi/scan').then((x) => x.json())
    return resp as ScanWifiNetworksRespBody
  }
}
