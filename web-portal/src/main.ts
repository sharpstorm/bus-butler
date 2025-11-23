document.addEventListener('DOMContentLoaded', () => {
  setTimeout(() => {
    const element = document.getElementById('test-element');
    if (!element) {
      return;
    }
    element.textContent = 'JS Loaded';
  }, 2000);
});
