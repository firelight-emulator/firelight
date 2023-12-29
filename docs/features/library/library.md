The user's library is a list of Entries that track the roms that the user has. Each Entry has a unique ID and references to the files that contain the rom data, as well as additional metadata and links to the appropriate ROMs, Games, Romhacks, etc, in the content database.

Since each Entry has these relationships to other well-defined data from the content database, we can perform queries like, "does the user have any version of Paper Mario?". Romhacks require very particular ROMs, and we can use this type of query to display information like: "You have Paper Mario in your library, but it's not the right version. You need this version: blah".  

We can also do things like track metrics for games and match users for multiplayer. Instead of just knowing "the user is playing `Super_Cool_Game_(USA) Rev 1.z64`", we know that the user is playing the game with ID `1234`, which in this case is `Super Cool Game`. That way, we have an easy data point that we can use to track all kinds of metrics about the games across multiple users. 

### Library database file

When the app starts, it checks for the existence of `library.db` in the X directory. If it's not found, it is created. T