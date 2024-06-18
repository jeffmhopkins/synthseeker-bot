# synthseeker-bot
Discord Bot for ninjam server, announces when users join and leaves the server. NEEDS updated Channel, Role, and TOKEN values.

Using dpp: https://dpp.dev/install-linux-deb.html

Using md5: https://github.com/stbrumme/hash-library/ (included)


# Install Notes

Install wget, and dpp.deb, clone this repo, and change directory into it:
```
apt install wget
wget -O dpp.deb https://dl.dpp.dev/
dpkg -i dpp.deb

git clone https://github.com/jeffmhopkins/synthseeker-bot/
cd synthseeker-bot/src
```
And then we edit the bot.cpp to add the discord token and server password etc:
```
nano bot.cpp
```
And then compile the binary:
```
make
```
This Produces the executable 'bot' in the same directory.
