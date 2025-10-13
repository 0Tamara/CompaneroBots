let note;
let octave;
let server_busy;

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


function pressKey(key)
{
    if(!busy)
    { 
        busy = 1;
        note = key.id.slice(4, 5)
        octave = key.id.slice(5)
        console.log(note + octave);
        fetch("/key-press?note=" + note + "&octave=" + octave)
            .then(response => response.text)
            .then(data =>
            {
                console.log(data);
                if(data == "1")
                    busy = 0;
            })
            .catch(error => 
            {
                console.log("!! ", error);
                busy = 0;
            });
    }
}