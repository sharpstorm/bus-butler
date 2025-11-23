export const findAndAssertDomId = <T extends HTMLElement>(
  elementId: string,
) => {
  const element = document.getElementById(elementId)
  if (!element) {
    throw new Error(`DOM Binding Error: Element not found "${elementId}"`)
  }
  return element as T
}
