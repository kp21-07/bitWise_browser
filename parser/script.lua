-- director script for LuaBrowser
-- This script will act as the "Director" for the UI.

print("[LUA] Hello from the Director script!")

-- 1. Try to find the header node via the new Search API
-- (In a real engine, we'd use getElemsByTag("header"), but we'll use ID 1 for now)
local header = browser.getElem(1)

if header then
    print("[LUA] Success! Found node ID: " .. header.id)
    print("[LUA] Current Background: " .. header.bgcolour)
else
    print("[LUA] Could not find header!")
end
