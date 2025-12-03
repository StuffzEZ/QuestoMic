# QuestoMic
Wireless microphone for quest using pico w

## BUILDING YOUR UF2
You will have to build your own UF2 for this. The generated UF2 is for testing purposes only and does not actually do anything.
1) Fork this repo
> [!TIP]
> You can make the repo private by forking your fork of this repo.
2) Change the wifi SSID and wifi password in uf2/main.c to your wifi password (line 24+25)
> [!WARNING]
> Your wifi password and SSID will be PUBLIC on the repo unless you have used the tip above to change it to a private repo
3) Create a new release in releases
4) You should see a .uf2 in there within ~5 minutes if everything worked!