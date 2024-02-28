"use strict";

// https://www.learnwithjason.dev/blog/get-form-values-as-json/


function handleSubmit(event) {
    // prevents default behavior
    event.preventDefault();

    // get form data
    const data = new FormData(event.target);

    // delete hidden enties
    var hiddenElements = document.querySelectorAll('.hidden');
    var dataNew = removeHiddenParameters(data, hiddenElements);

    const value = Object.fromEntries(dataNew.entries());
    
        // Inhalt und Größe von Value ausgeben 
        console.log(value);
        var valueSize = Object.keys(value).length;
        console.log(valueSize); // 45



    // JSON-Data in string
    var jsonData = JSON.stringify(value);    

    // XMLHttpRequest 
    var xhr = new XMLHttpRequest();
    var url = "/api/addprofile"; 
    
    xhr.open("POST", url, true);
    xhr.setRequestHeader("Content-Type", "application/json");

    // Funktion, die aufgerufen wird, wenn die Anfrage abgeschlossen ist
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            // Erfolgreiche Verarbeitung auf der Serverseite
            console.log("Daten erfolgreich gesendet.");
            window.location.href = 'index.html';
        }
    };

    // Anfrage senden
    xhr.send(jsonData);
}

// Removes the parameters in the form data, that were hidden
function removeHiddenParameters(formData, hiddenElements) {
    // iterates through elements that have the class "hidden"
    hiddenElements.forEach(function(hiddenElement) {
        // deletes every input that is hidden
        var inputElements = hiddenElement.querySelectorAll('input');
        if (inputElements) {
            inputElements.forEach(function(inputElement) {
                var inputName = inputElement.name;
                console.log(inputName);
                formData.delete(inputName);
            })
        }
        // deletes every select that is hidden
        var selectElements = hiddenElement.querySelectorAll('select');
        if (selectElements) {
            selectElements.forEach(function(selectElement) {
                var selectName = selectElement.name;
                console.log(selectName);
                formData.delete(selectName);
            })
        }
    });
    console.log(formData);

    return formData;
}

// Changes the visibility of the sections
function changeVisibility(checkbox, layerNum) {
    // find next section
    var section = findNextSection(checkbox);
   
    if (checkbox.checked) {
        console.log("Checkbox is checked..");
        // for (let i = 0; i < elements.length; i++) {
        //     elements.item(i).classList.remove("hidden");
        // }
        section.classList.remove("hidden");
    } else {
        console.log("Checkbox is not checked..");
        // for (let i = 0; i < elements.length; i++) {
        //     elements.item(i).classList.add("hidden");
        // }
        section.classList.add("hidden");
    }
}

function findNextSection(element) {
    var nextSibling = element.parentElement.nextElementSibling;

    // Durchlaufe die Geschwisterelemente, bis ein <section>-Element gefunden wird
    while (nextSibling) {
        if (nextSibling.tagName === 'SECTION') {
            return nextSibling; // Gib das gefundene <section>-Element zurück
        }
        nextSibling = nextSibling.nextElementSibling;
    }

    return null; // Kein <section>-Element gefunden
}


document.addEventListener("DOMContentLoaded", function () {

    // Listener on every checkbox - changes section visibility
    for(var i = 0; i < 2; i++) {
        var checkboxList =  document.querySelectorAll(`input[type="checkbox"].layer-${i}`);
        console.log(checkboxList);
        // var releventCheckboxes = checkboxList.querySelectorAll('.layer-'.i);
        checkboxList.forEach(function(checkbox) {
            checkbox.addEventListener('change', function() {
                changeVisibility(checkbox, i)
            })
        })
    }
        // var inputLayerZero = document.querySelectorAll('input[type="checkbox"]').querySelectorAll('.layer-0');
        // inputLayerZero.forEach(function(input) {
        //     input.addEventListener('change', function() {
        //         changeVisibility(input, layer1)
        //     })
        // })
        // var inputLayerOne = document.querySelectorAll('input[type="checkbox"]').querySelectorAll('.layer-1');
        // inputLayerOne.forEach(function(input) {
        //     input.addEventListener('change', function() {
        //         changeVisibility(input, layer1)
        //     })
        // })
        // var checkbox = document.getElementById('coffeeProfile__form-PI-state-ID');
        // var elements = document.getElementsByClassName('coffeeProfile__form-PI--layer-1');

        // var checkbox = document.getElementById('coffeeProfile__form-SK-state-ID');
        // var elements = document.getElementsByClassName('coffeeProfile__form-SK--layer-1');
    

        // checkbox.addEventListener('change', function() {
        //     changeVisibility(checkbox, elements)} ); 



        // var checkboxList = document.getElementsByTagName('')
            
    
        
    // Listener for form submit
    document.querySelector("form").addEventListener("submit", handleSubmit);
});

