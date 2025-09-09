document.addEventListener('DOMContentLoaded', () => {
    // Open File Explorer Window
    const fileExplorerBtn = document.getElementById('file-explorer-btn');
    const fileExplorerWindow = document.getElementById('file-explorer-window');
    const closeFileExplorer = document.getElementById('close-file-explorer');
    
    fileExplorerBtn.addEventListener('click', () => {
        fileExplorerWindow.style.display = 'block';
    });

    closeFileExplorer.addEventListener('click', () => {
        fileExplorerWindow.style.display = 'none';
    });

    // Draggable Window Logic
    let isDragging = false;
    let offsetX, offsetY;
    const windowHeader = document.querySelector('.window-header');
    
    windowHeader.addEventListener('mousedown', (e) => {
        isDragging = true;
        offsetX = e.clientX - fileExplorerWindow.offsetLeft;
        offsetY = e.clientY - fileExplorerWindow.offsetTop;
    });

    windowHeader.addEventListener('mousemove', (e) => {
        if (isDragging) {
            fileExplorerWindow.style.left = `${e.clientX - offsetX}px`;
            fileExplorerWindow.style.top = `${e.clientY - offsetY}px`;
        }
    });

    windowHeader.addEventListener('mouseup', () => { isDragging = false; });
});
