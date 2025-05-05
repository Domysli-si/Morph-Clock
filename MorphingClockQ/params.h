//wifi network settings
char wifi_ssid[] = "Tenda_834708";       
char wifi_pass[] = "Kacenka11";        

//Time settings
char timezone[] = "2" ;   
char military[] = "N";     
char date_fmt[] = "D.M.Y"; // date format: D.M.Y or M.D.Y or M.D or D.M or D/M/Y
char dst_sav[] = "true";   //Daylight savings time

// RGB Maxtix settings
char c_palet[] = "1";      
char brightness[] = "60";  //Brightness  (Don't go over 70)

char nightStart[] = "23";    
char nightEnd[] = "7";       


char weatherservice[] = "4";         
char apiKey[]         = "";          
char postal_code[]    = "";          
char country_code[]   = "";          

char latitude[]       = "50.0755";          
char longitude[]      = "14.4378";          

char u_metric[] = "Y";              
char w_animation[] = "Y";           
char wdefine[] = "";                


String apikeys[8] = {"",    //WeatherAPI
                     "",    //WeatherBit
                     "",    //PirateWeather
                     "",    //OpenMeteo - not required
                     "",    //WeatherUnlocked
                     "",    //future weather service
                     ""};   //Accuweather (Currently not working)
