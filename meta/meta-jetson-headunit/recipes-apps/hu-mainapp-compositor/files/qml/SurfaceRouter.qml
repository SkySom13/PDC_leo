import QtQuick 2.12

/*
 * Surface Router - Logic for routing client app windows to containers
 * Routes based on app_id or window title
 */

QtObject {
    id: router

    // Container references (set by compositor)
    property var gearAppContainer: null
    property var homeScreenAppContainer: null
    property var mediaAppContainer: null
    property var ambientAppContainer: null

    // Get target container without actually routing
    function getTargetContainer(identifier) {
        var idLower = identifier.toLowerCase()
        
        if (identifier === "GearApp" || idLower.includes("gear")) {
            return gearAppContainer
        } else if (identifier === "HomeScreenApp" || idLower.includes("homescreen") || idLower.includes("home screen")) {
            return homeScreenAppContainer
        } else if (identifier === "MediaApp" || idLower.includes("media")) {
            return mediaAppContainer
        } else if (identifier === "AmbientApp" || idLower.includes("ambient")) {
            return ambientAppContainer
        } else {
            // Default: Home page
            return homeScreenAppContainer
        }
    }

    // Helper function to clear container and add new surface
    function assignToContainer(chrome, container, containerName) {
        // Clear any existing children in the container (prevent duplicates)
        for (var i = container.children.length - 1; i >= 0; i--) {
            var child = container.children[i]
            if (child !== chrome) {
                console.log("   ⚠️  Removing existing surface from", containerName)
                child.visible = false
                child.parent = null
            }
        }

        // Assign new surface
        chrome.parent = container
        chrome.anchors.fill = chrome.parent
        chrome.visible = true
        console.log("   → " + containerName + " ✅")
    }

    // Route surface to appropriate container
    function routeSurface(chrome, identifier) {
        if (!chrome) {
            console.error("❌ Cannot route: chrome is null")
            return
        }

        var idLower = identifier.toLowerCase()

        console.log("🔀 Routing surface...")
        console.log("   Identifier:", identifier)

        // Route by app_id or window title
        if (identifier === "GearApp" || idLower.includes("gear")) {
            if (gearAppContainer) {
                assignToContainer(chrome, gearAppContainer, "Left Gear Panel")
            } else {
                console.error("   ❌ gearAppContainer is null!")
            }
            return

        } else if (identifier === "HomeScreenApp" || idLower.includes("homescreen") || idLower.includes("home screen")) {
            if (homeScreenAppContainer) {
                assignToContainer(chrome, homeScreenAppContainer, "Home Page")
            } else {
                console.error("   ❌ homeScreenAppContainer is null!")
            }
            return

        } else if (identifier === "MediaApp" || idLower.includes("media")) {
            if (mediaAppContainer) {
                assignToContainer(chrome, mediaAppContainer, "Media Page")
            } else {
                console.error("   ❌ mediaAppContainer is null!")
            }
            return

        } else if (identifier === "AmbientApp" || idLower.includes("ambient")) {
            if (ambientAppContainer) {
                assignToContainer(chrome, ambientAppContainer, "Ambient Page")
            } else {
                console.error("   ❌ ambientAppContainer is null!")
            }
            return

        // Test client routing
        } else if (identifier === "test_gearapp" || (idLower.includes("test") && idLower.includes("gear"))) {
            if (gearAppContainer) {
                assignToContainer(chrome, gearAppContainer, "Left Gear Panel (test)")
            }
            return

        } else if (identifier === "test_homescreenapp" || (idLower.includes("test") && idLower.includes("home"))) {
            if (homeScreenAppContainer) {
                assignToContainer(chrome, homeScreenAppContainer, "Home Page (test)")
            }
            return

        } else if (identifier === "test_mediaapp" || (idLower.includes("test") && idLower.includes("media"))) {
            if (mediaAppContainer) {
                assignToContainer(chrome, mediaAppContainer, "Media Page (test)")
            }
            return

        } else if (identifier === "test_ambientapp" || (idLower.includes("test") && idLower.includes("ambient"))) {
            if (ambientAppContainer) {
                assignToContainer(chrome, ambientAppContainer, "Ambient Page (test)")
            }
            return
        }

        // Default: Don't route unknown surfaces (prevents overlapping)
        console.error("⚠️  Unknown app_id - NOT ROUTING:", identifier)
        console.error("   Surface will be hidden. Check app's QGuiApplication::setApplicationName()")

        // Hide the chrome so it doesn't appear anywhere
        chrome.visible = false
    }
}
