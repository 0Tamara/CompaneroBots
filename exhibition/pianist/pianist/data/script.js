let note;
let octave;
let server_busy;
let song;

const keyboardBtn = document.getElementById('keyboardBtn');
const songsBtn = document.getElementById('songsBtn');
const pianoContainer = document.getElementById('pianoContainer');
const songsContainer = document.getElementById('songsContainer');
const songs = document.querySelectorAll('.songs-list li');

pianoContainer.style.display = 'block';
songsContainer.style.display = 'none';

window.onload = function() {
  loadSongInfo();
};

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
    if(!server_busy)
    { 
        server_busy = 1;
        note = key.id.slice(4, 5)
        octave = key.id.slice(5)
        console.log("play note " + note + octave);
        // zakliknutie tlacitka
        fetch("/key-press?note=" + note + "&octave=" + octave)
            .then(response => response.text())
            .then(data =>
            {
                console.log("Repply: " + data);
                if(data === "done")
                {
                    // odkliknutie tlacitka
                    server_busy = 0;
                }
            })
            .catch(error => console.log("!! ", error));
    }
}

function pressSong(key)
{
    if(!server_busy)
    { 
        server_busy = 1;
        song = key.id.slice(5)
        console.log("play song " + song);
        // zakliknutie tlacitka
        fetch("/play-song?song=" + song)
            .then(response => response.text())
            .then(data =>
            {
                console.log("Repply: " + data);
                if(data === "done")
                {
                    // odkliknutie tlacitka
                    server_busy = 0;
                }
            })
            .catch(error => console.log("!! ", error));
        
        // Start polling for status 2x a second
        /*const statusInterval = setInterval(() =>
        {
            fetch("/status")
                .then(response => response.text())
                .then(status =>
                {
                    console.log("Status:", status);
                    if (status === "done")
                    {
                        // odkliknutie tlacitka
                        clearInterval(statusInterval);
                        server_busy = 0;
                    }
                })
                .catch(error => console.log("!! ", error));
        }, 500);*/
    }
}

function loadSongInfo()
{
    let song_names;
    fetch("/songs-info")
        .then(response => response.text())
        .then(data =>
        {
            console.log("Message:", data);
            song_names = data.split(",");
            for(let i=0; i<song_names.length; i++)  
            {
                document.getElementById("song_" + (i+1)).innerHTML = song_names[i];
            }
        })
        .catch(error => console.log("!! ", error));
}