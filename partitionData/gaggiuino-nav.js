"use strict";

const gaggiuinoLogo = "Gaggiuino-Logo.png";
const gaggiuinoLogoLink = "index.html";

const gaggiuinoNavItems = [
    ["Profile", "index.html#profileOverviewID"],
    ["Dashboard", "index.html"],
    ["About Us", "index.html#aboutUsID"],
    ["Login", "index.html"]
];

function gaggiuinoCreateNav(logo, logoLink, items) {
    
    let headerDiv = document.createElement("div");
    headerDiv.classList.add("header__div");

    let headerLogo = document.createElement("a");
    headerLogo.href = logoLink;
    headerLogo.classList.add("header__logo-link");
    headerLogo.width = 500;
    let headerImg = document.createElement("img");
    headerImg.src = logo;
    headerImg.classList.add("header__logo");
    headerImg.alt = "Gaggiuino-Logo";
    headerImg.width = 500;

    headerLogo.appendChild(headerImg);
    

    let headerNav = document.createElement("nav");
    headerNav.classList.add("header__nav");

    let headerNavUl = document.createElement("ul");
    
    items.forEach(function (item) {
        console.log("Hallo");
        let li = document.createElement("li");
        let liLink = document.createElement("a");
        liLink.textContent = item[0];
        liLink.href = item[1];

        li.appendChild(liLink);
        headerNavUl.appendChild(li);
    });

    headerNav.appendChild(headerNavUl);

    headerDiv.appendChild(headerLogo);
    headerDiv.appendChild(headerNav);

    document.getElementById("gaggiuino-header").appendChild(headerDiv);
}

window.addEventListener("load", (event) => {
    gaggiuinoCreateNav(gaggiuinoLogo, gaggiuinoLogoLink, gaggiuinoNavItems);
});
