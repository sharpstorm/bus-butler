import { SetupState } from './setup-state'
import { setAppState } from './state'

let setupState: SetupState | undefined

document.addEventListener('DOMContentLoaded', () => {
  setupState = new SetupState()
  setupState.onWorkflowComplete(() => {
    setAppState('config')
  })
  setAppState('setup')
})
