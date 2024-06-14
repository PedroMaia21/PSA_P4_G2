/* ------------------------------------------------------
    Connection to the WebSockets that connect to node-red
-------------------------------------------------------*/
const socket = new WebSocket('ws://localhost:1880/ws/PSA24G2');
const socketPos = new WebSocket('ws://localhost:1880/ws/PSA24G2/lastPos')
const socketHist = new WebSocket('ws://localhost:1880/ws/PSA24G2/hist')
const socketHistReq = new WebSocket('ws://localhost:1880/ws/PSA24G2/histReq')
const socketPoints = new WebSocket('ws://localhost:1880/ws/PSA24G2/points')
const socketRequest = new WebSocket('ws://localhost:1880/ws/PSA24G2/request')
const socketState = new WebSocket('ws://localhost:1880/ws/PSA24G2/state')
const socketError = new WebSocket('ws://localhost:1880/ws/PSA24G2/error')


socket.onopen = function(event) {
    console.log('WebSocket connection established');
    const canvas = document.getElementById("graph");
    const ctx = canvas.getContext("2d");
    ctx.font = "10px Arial";
    ctx.fillText("Clique numa linha da tabela de histórico",50,50);
    ctx.fillText("para analisar as movimentações realizadas",50,65);
};

socket.onclose = function(event) {
    console.log('WebSocket connection closed');
};

socket.onmessage = function(event) {
    const message = event.data;
    console.log('Message received from server:', message);
};

socket.onerror = function(error) {
    console.error('WebSocket error:', error);
};

function sendMessage(message) {
    setTimeout(() => { 
        socket.send(message);
        console.log('Message sent to server:', message); 
    }, 500);           
};

function sendMessageHist(message) {
    setTimeout(() => { 
        socketHistReq.send(message);
        console.log('Message sent to server:', message); 
    }, 500);           
};



/* ----------------------------------------------------------------------------
    Callback Function for the call to get the last known position of the gantry
---------------------------------------------------------------------------- */
socketPos.onmessage = function(event) {
    const message = event.data;
    const data = JSON.parse(message);

    if (Array.isArray(data) && data.length > 0) {
        const firstObject = data[0];
        // Update the HTML table with the received data
        const table = document.getElementById('tablePos');
        if (table) {
            // Assuming the table has three columns for X, Y, and Z values
            if (firstObject.hasOwnProperty('X') && firstObject.hasOwnProperty('Y') && firstObject.hasOwnProperty('Z')) {
                table.rows[1].cells[0].textContent = firstObject.X;
                table.rows[1].cells[1].textContent = firstObject.Y;
                table.rows[1].cells[2].textContent = firstObject.Z;

                const xRange = document.getElementById("X_range_1")
                const yRange = document.getElementById("Y_range_1")
                const zRange = document.getElementById("Z_range_1")

                const xNumber = document.getElementById("X_input_1")
                const yNumber = document.getElementById("Y_input_1")
                const zNumber = document.getElementById("Z_input_1")
        
                xRange.value = firstObject.X
                yRange.value = firstObject.Y
                zRange.value = firstObject.Z
                
                xNumber.placeholder = firstObject.X
                yNumber.placeholder = firstObject.Y
                zNumber.placeholder = firstObject.Z

            } else {
                // If message doesn't have expected properties, show placeholder values
                table.rows[1].cells[0].textContent = 'Error';
                table.rows[1].cells[1].textContent = 'Error';
                table.rows[1].cells[2].textContent = 'Error';
            }
        }
    } else {
        console.error('Invalid message format:', message);
    }
};
/* ------------------------------------------------------------------------------
    Callback function to update if there are any updates in the table of requests
------------------------------------------------------------------------------ */
socketHist.onmessage = function(event) {
    const message = event.data;
    const data = JSON.parse(message);
    generatePaginationButtons(data[0]['ID'])

    if (Array.isArray(data) && data.length > 0) {
        // Update the HTML table with the received data
        const table = document.getElementById('tableHist');
        var cells = document.querySelectorAll('#tableHist td')
        cells.forEach(function(cell, index) {
            cell.innerHTML = '-';
        });
        for (let i=0; i<data.length; i++){
            const object = data[i];
            if(table){
                if(object.hasOwnProperty('mode') && object.hasOwnProperty('state') && object.hasOwnProperty('time_r')){
                    const time_r = new Date(object.time_r).toLocaleString();
                    if (object.state==1){
                        stateStr='Concluído'
                    } else {
                        stateStr='Interrompido'
                    }
                    table.rows[i+1].cells[0].textContent = object.ID
                    table.rows[i+1].cells[1].textContent = object.mode;
                    table.rows[i+1].cells[2].textContent = time_r;
                    table.rows[i+1].cells[3].textContent = stateStr;
                } else {
                    table.rows[i+1].cells[0].textContent = 'Error';
                    table.rows[i+1].cells[1].textContent = 'Error';
                    table.rows[i+1].cells[2].textContent = 'Error';
                }
            }
        }        
    } else {
        console.error('Invalid message format: ', message);
    }
};
/* ----------------------------------------------------------------------------------------------
    Function that allows clicking on the table and sends the clicked table data to get its points
---------------------------------------------------------------------------------------------- */
setTimeout(() => { 
    const tableHist = document.getElementById("tableHist");
    var rowsTH = tableHist.getElementsByTagName("tr");
    for (var i = 0; i < rowsTH.length; i++) {
        var row = rowsTH[i];
        row.addEventListener("click", function() {
            // Call your JavaScript function here
            rowClicked(this);
        });
    }
}, 2000); 
function rowClicked(row) {
    setTimeout(() => { 
        socketPoints.send(row.cells[0].textContent);
        console.log('Message sent to server:', row.cells[0].textContent); 
    }, 500); 
}
/* ----------------------------------------------------------------------------
    Callback function that draws on the canvas the graphs of the points history
---------------------------------------------------------------------------- */
socketPoints.onmessage = function(event) {
    var canvas = document.getElementById('graph');
    var ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.font = "10px Arial";
    var data = event.data
    data = JSON.parse(data)

    // ----- Separates the data sent into arrays ----- //
    function parseData(data){
        var positions = {}
        data.forEach(function(entry){
            entry.forEach(function(point){
                if (point.X !== undefined){
                    if (!positions[point.timeX]){
                        positions[point.timeX] = { X: point.X };
                    } else {
                        positions[point.timeX].X = point.X;
                    }
                }
                if (point.Y !== undefined && point.Z !== undefined) {
                    if (!positions[point.timeYZ]) {
                        positions[point.timeYZ] = { Y: point.Y, Z: point.Z };
                    } else {
                        positions[point.timeYZ].Y = point.Y;
                        positions[point.timeYZ].Z = point.Z;
                    }
                }
            })
        })
        return positions;
    }

    var positions = parseData(data);
    var minTime = Infinity;
    var maxTime = -Infinity;
    var minX = Infinity;
    var minY = Infinity;
    var minZ = Infinity;
    var maxX = -Infinity;
    var maxY = -Infinity;
    var maxZ = -Infinity;

    // ----- Get the max an min for each axis ----- //
    Object.keys(positions).forEach(function(time) {
        var point = positions[time];
        minTime = Math.min(minTime, new Date(time).getTime());
        maxTime = Math.max(maxTime, new Date(time).getTime());
        if (point.X !== undefined) {
            minX = Math.min(minX, point.X);
            maxX = Math.max(maxX, point.X);
        }
        if (point.Y !== undefined) {
            minY = Math.min(minY, point.Y);
            maxY = Math.max(maxY, point.Y);
        }
        if (point.Z !== undefined) {
            minZ = Math.min(minZ, point.Z);
            maxZ = Math.max(maxZ, point.Z);
        }
    });
    minG = Math.min(minX, minY, minZ)
    maxG = Math.max(maxX, maxY, maxZ)

    function mapValueToCanvas(value, min, max, canvasSize) {
        return (value - min) / (max - min) * canvasSize;
    }

    // ----- Define the colors for X, Y, and Z lines -----
    var colors = ["red", "green", "blue"];

    // ----- Function to draw grid lines and labels -----
    function drawGrid(ctx, canvas) {
        
        ctx.fillText('Histórico de Posições',(canvas.width/2) - 50, 10)
        ctx.strokeStyle='grey'
        
        var margin = 25;

        var stepX = (canvas.width - 2 * margin) / 5;
        var stepY = (canvas.height - 2 * margin) / 5;

        // Draw horizontal grid lines
        n=0;
        for (var i = 0; i <= canvas.height - 2 * margin; i += stepY) {
            ctx.beginPath();
            ctx.moveTo(margin, i + margin);
            ctx.lineTo(canvas.width - margin, i + margin);
            ctx.stroke();

            // Draw Y-axis labels
            ctx.fillText((n*((maxG - minG) / 5) + minG).toFixed(0), 5, 5*stepY - i + margin - 5);
            n += 1
        }

        var timeDifference = (maxTime - minTime) / (canvas.width - 2 * margin);

        // Draw vertical grid lines
        for (var i = 0; i <= canvas.width - 2 * margin; i += stepX) {
            ctx.beginPath();
            ctx.moveTo(i + margin, margin);
            ctx.lineTo(i + margin, canvas.height - margin);
            ctx.stroke();
    
            // Draw X-axis labels
            var timeLabel = ((i * timeDifference)/100).toFixed(0);
            ctx.fillText(timeLabel, i + margin, canvas.height - 15);  
            ctx.fillText('Tempo [s]', (canvas.width/2)-25, canvas.height - 5);      
        }
    }

    // Function to draw lines
    function drawLines(ctx, positions, minX, maxX, minY, maxY, minZ, maxZ, minTime, maxTime, canvas) {
        var margin = 25;
        var minG = Math.min(minX, minY, minZ);
        var maxG = Math.max(maxX, maxY, maxZ);
        count=0
        // Draw X line
        ctx.beginPath();
        ctx.strokeStyle = colors[0]; // Set color for X line
        Object.keys(positions).forEach(function(time) {
            var point = positions[time];
            var x = mapValueToCanvas(new Date(time).getTime(), minTime, maxTime, canvas.width - 2 * margin) + margin;
            var yX = point.X !== undefined ? canvas.height - mapValueToCanvas(point.X, minG, maxG, canvas.height - 2 * margin) - margin : NaN;
            if (!isNaN(yX)) {
                if (count == 0) {
                    ctx.moveTo(x, yX);
                    count += 1
                } else {
                    ctx.lineTo(x, yX);
                }
            }
        });
        ctx.lineWidth = 2;
        ctx.stroke();
    
        // Draw Y line
        ctx.beginPath();
        ctx.strokeStyle = colors[1]; // Set color for Y line
        count=0
        Object.keys(positions).forEach(function(time) {
            var point = positions[time];
            var x = mapValueToCanvas(new Date(time).getTime(), minTime, maxTime, canvas.width - 2 * margin) + margin;
            var yY = point.Y !== undefined ? canvas.height - mapValueToCanvas(point.Y, minG, maxG, canvas.height - 2 * margin) - margin : NaN;
            if (!isNaN(yY)) {
                if (count == 0) {
                    ctx.moveTo(x, yY);
                    count += 1;
                } else {
                    ctx.lineTo(x, yY);
                }
            }
        });
        ctx.lineWidth = 2;
        ctx.stroke();
    
        // Draw Z line
        ctx.beginPath();
        ctx.strokeStyle = colors[2]; // Set color for Z line
        count = 0
        Object.keys(positions).forEach(function(time) {
            var point = positions[time];
            var x = mapValueToCanvas(new Date(time).getTime(), minTime, maxTime, canvas.width - 2 * margin) + margin;
            var yZ = point.Z !== undefined ? canvas.height - mapValueToCanvas(point.Z, minG, maxG, canvas.height - 2 * margin) - margin : NaN;
            if (!isNaN(yZ)) {
                if (count == 0) {
                    ctx.moveTo(x, yZ);
                    count += 1
                } else {
                    ctx.lineTo(x, yZ);
                }
            }
        });
        ctx.lineWidth = 2;
        ctx.stroke();
    }
    

    // Call the drawGrid function before drawing lines
    drawGrid(ctx, canvas);

    // Call the drawLines function to draw X, Y, and Z lines
    drawLines(ctx, positions, minX, maxX, minY, maxY, minZ, maxZ, minTime, maxTime, canvas);

    console.log('canvas updated');
}
/* -----------------------------------------------------------------------------
    Function that keeps the number input and the slide bar updated to each other
----------------------------------------------------------------------------- */
document.addEventListener('DOMContentLoaded', function () {
    // Get all range and number inputs
    const rangeInputs = document.querySelectorAll('input[type="range"]');
    const numberInputs = document.querySelectorAll('input[type="number"]');
    
    // Function to sync range and number inputs
    function syncInputs(event) {
        const input = event.target;
        const value = input.value;
        const id = input.id;

        // Update corresponding number input
        const correspondingNumberInput = document.querySelector(`input[type="number"][id="${id.replace('range', 'input')}"]`);
        correspondingNumberInput.value = value;

        // Update corresponding range input
        const correspondingRangeInput = document.querySelector(`input[type="range"][id="${id.replace('input', 'range')}"]`);
        if (correspondingRangeInput) {
            correspondingRangeInput.value = value;
        }
    }
    
    // Attach event listeners to range inputs
    rangeInputs.forEach(input => {
        input.addEventListener('input', syncInputs);
    });
    
    // Attach event listeners to number inputs
    numberInputs.forEach(input => {
        input.addEventListener('input', syncInputs);
    });
    console.log('loaded')
});

function sendZeroMode(){
    console.log('Modo zero')
    socketRequest.send('M0XIYIZIXFYFZFN')
    changeState('moving')
}

var modeToSend = 2
function changeMode(mode){
    modeToSend=mode
    document.getElementById("controlPanel").classList.remove("hidden");
    document.getElementById("controlPanelPlaceholder").classList.add("hidden");
    if (mode==1) {
        document.getElementById("mode2").classList.add("hidden");
    } else if (mode==2) {
        document.getElementById("mode2").classList.remove("hidden");
        
    }
}

function sendRequest(){
    var mode=modeToSend
    var X1=document.getElementById('X_range_1').value
    var Y1=document.getElementById('Y_range_1').value
    var Z1=document.getElementById('Z_range_1').value
    var X2=document.getElementById('X_range_2').value
    var Y2=document.getElementById('Y_range_2').value
    var Z2=document.getElementById('Z_range_2').value
    var n=document.getElementById('n_cycles').value
    message='M'+mode+'XI'+X1+'YI'+Y1+'ZI'+Z1+'XF'+X2+'YF'+Y2+'ZF'+Z2+'N'+n
    socketRequest.send(message)
    changeState('moving')
}

var executed=false
function generatePaginationButtons(numberOfRequests){
    if(!executed){
        const totalPages = Math.ceil(numberOfRequests/10)
        const paginationContainer = document.getElementById('pagination')
        paginationContainer.innerHTML=''
        if (totalPages<6){
            for (let i = 1; i <= totalPages; i++) {
                const button = document.createElement('button');
                button.textContent = i;
                button.classList.add('pageButton')
                button.addEventListener('click', function () {
                    socketHistReq.send(i);
                    highlightButton(i);
                });
                paginationContainer.appendChild(button);
            }
        } else{
            for (let i = 1; i <= 4; i++) {
                const button = document.createElement('button');
                button.textContent = i;
                button.classList.add('pageButton')
                button.addEventListener('click', function () {
                    socketHistReq.send(i);
                    changeButtons(i,totalPages);
                    highlightButton(i);
                });
                paginationContainer.appendChild(button);
            }
            const button = document.createElement('button');
            button.textContent = totalPages;
            button.classList.add('pageButton')
            button.addEventListener('click', function () {
                socketHistReq.send(totalPages);
                changeButtons(totalPages,totalPages);
                highlightButton(totalPages);
            });
            paginationContainer.appendChild(button);
        }
    }
    executed=true
}

function highlightButton(pageNumber) {
    const buttons = document.querySelectorAll('#pagination button');
    buttons.forEach(button => {
        button.classList.remove('active');
        if (parseInt(button.textContent) === pageNumber) {
            button.classList.add('active');
        }
    });
}

function changeButtons(current, total){
    const paginationContainer = document.getElementById('pagination')
    paginationContainer.innerHTML=''
    if (current<3){
        var buttonsToShow = [1, 2, 3, 4, total];
    } else if (total-current<2){
        var buttonsToShow = [1, total-3, total-2, total-1, total];
    } else {
        var buttonsToShow = [1, current - 1, current, current + 1, total];
    }
    console.log(buttonsToShow)
    buttonsToShow.forEach(pageNumber => {
        if (pageNumber >= 1 && pageNumber <= total) {
            const button = document.createElement('button');
            button.textContent = pageNumber;
            button.classList.add('pageButton')
            button.addEventListener('click', function () {
                socketHistReq.send(pageNumber);
                changeButtons(pageNumber,total);
                highlightButton(pageNumber);
            });
            paginationContainer.appendChild(button);
        }
    });
}

function changeState(state){
    var loadercontainer = document.getElementById("loading-container")
    var form = document.getElementById("controlPanel")
    var list = document.getElementById("modeList")
    if (state=='moving'){
        loadercontainer.classList.remove('hidden')
        form.classList.add("hidden")
        list.classList.add("hidden")
        document.getElementById('state').innerHTML="Em movimento"
        shouldContinue = true;
        updateCurrentPosition();
    } else {
        loadercontainer.classList.add('hidden')
        form.classList.remove("hidden")
        list.classList.remove("hidden")
        document.getElementById('state').innerHTML="Parado"
        shouldContinue = false;
    }
}

socketState.onmessage = function(event) {
    changeState('stopped')
};

let shouldContinue = true;
let timeoutID
function updateCurrentPosition(){
    sendMessage('Get Data');
    if (!shouldContinue) {
        clearTimeout(timeoutID);
    }else{
        timeoutID = setTimeout(updateCurrentPosition, 5000);
    }
}

function randomizeInputs() {
    const inputs = document.querySelectorAll('#controlPanel input[type="number"]');
    inputs.forEach(input => {
        if (input.id !== 'n_cycles') {
            const min = parseInt(input.min);
            const max = parseInt(input.max);
            randomValue = Math.floor(Math.random() * (max - min + 1)) + min;
            input.value = randomValue;
            document.getElementById(input.id.replace('input', 'range')).value = randomValue;
        }
    });
}

socketError.onmessage = function(event) {
    const message = event.data;
    console.log('Message received from server:', message);
    window.alert("Ocorreu um erro - Pórtico vai voltar para o zero");
};