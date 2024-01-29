# Show your wakatime on OLED display using an ESP32 Dev Module
![Demo](/images/board.jpg)

#### First you need to create an account on wakatime and connect it with VSCode then you need to create wakapi account and change the API key, and add an API URL into your ~./wakatime.cfg. 
#### The time Updates every 3 minutes. It can be changed by changing the requset_interval variable. 
## Important 
#### In order for wakapi to work you need to go to settings > permissions and set the time range to 1, or increase it accordingly to how you want to access your data, but it can't be 0
## Wiring for the board
![Wiring](/images/wiring.jpg)