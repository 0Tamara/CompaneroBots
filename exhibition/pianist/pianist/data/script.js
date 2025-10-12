const keyboardBtn = document.getElementById('keyboardBtn');
const songsBtn = document.getElementById('songsBtn');
const pianoContainer = document.getElementById('pianoContainer');
const songsContainer = document.getElementById('songsContainer');
const songs = document.querySelectorAll('.songs-list li');

pianoContainer.style.display = 'block';
songsContainer.style.display = 'none';

keyboardBtn.addEventListener('click', () => {
    pianoContainer.style.display = 'block';
    songsContainer.style.display = 'none';
    keyboardBtn.classList.add('active');
    songsBtn.classList.remove('active');
});

songsBtn.addEventListener('click', () => {
    pianoContainer.style.display = 'none';
    songsContainer.style.display = 'block';
    songsBtn.classList.add('active');
    keyboardBtn.classList.remove('active');
});

songs.forEach(song => {
    song.addEventListener('click', () => {
        if (song.classList.contains('selected')) {
            song.classList.remove('selected');
        } else {
            songs.forEach(s => s.classList.remove('selected'));
            song.classList.add('selected');
        }
    });
});
