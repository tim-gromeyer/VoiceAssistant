const featureItems = document.querySelectorAll(".feature-item");

const darkModeToggle = document.getElementById("dark-mode-toggle");
const prefersDarkMode = window.matchMedia("(prefers-color-scheme: dark)").matches;

darkModeToggle.addEventListener("click", () => {
    const isDarkMode = document.body.classList.toggle("dark-mode");
    const darkModeToggleEmoji = darkModeToggle.innerText;

    // Update the dark mode toggle button emoji based on the mode
    if (isDarkMode) {
        darkModeToggle.innerText = "🌞"; // Set sun emoji
    } else {
        darkModeToggle.innerText = "🌙"; // Set moon emoji
    }

    featureItems.forEach((item) => item.classList.toggle("dark-mode"));
});

if (prefersDarkMode) {
    darkModeToggle.click();
}

// Hamburger menu for small screens
const hamburger = document.querySelector(".hamburger");
const navMenu = document.querySelector(".nav-menu");
const navLink = document.querySelectorAll(".nav-link");

hamburger.addEventListener("click", mobileMenu);
navLink.forEach((n) => n.addEventListener("click", closeMenu));

function mobileMenu() {
    hamburger.classList.toggle("active");
    navMenu.classList.toggle("active");
}

function closeMenu() {
    hamburger.classList.remove("active");
    navMenu.classList.remove("active");
}
