"use strict"

function buildProfileOverview() {
    // First GET-Request to get Ids
    var xhr = new XMLHttpRequest();
    var url = "/api/getprofileids"; 
        
    xhr.open("GET", url, true);
    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            console.log("Ids erfolgreich angefragt.");
            profileIds = xhr.response;

            console.log(profileIds)

            // Second GET-Request to get the Names
            // profileIds.value.forEach(id => {
            //     var xhr = new XMLHttpRequest();
            //     var url = `/api/getprofile?id=${id}`; 
                    
            //     xhr.open("GET", url, true);
            //     xhr.setRequestHeader("Content-Type", "application/json");

            //     xhr.onreadystatechange = function () {
            //         if (xhr.readyState === 4 && xhr.status === 200) {
            //             console.log("Daten erfolgreich angefragt.");
            //             profileData = xhr.response;
            //         }
            //     };
            //     xhr.send();

            //     console.log(profileData.name);
            // });
        }
    };
    xhr.send();
}

window.addEventListener("load", (event) => {
    buildProfileOverview();});
