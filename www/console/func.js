var ws = new WebSocket("wss://" + window.location.host + "/ws/");
ws.onmessage = function(msg){
    document.getElementById("console_text").innerHTML += msg.data;
}

async function console_input_enter(event) {
    if (event.keyCode == 13) {
        const text = document.getElementById("console_text");
        const input = document.getElementById("console_input");
        const box = document.getElementById("console_window");

        ws.send(input.value + "<br>");
        input.value = "";

        box.scrollTop = box.scrollHeight;
    }
}


