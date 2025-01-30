# SEChat
*DE*: SEChat ist ein Chatraum CLI-Tool, mithilfe dessen Chatnachrichten verschlüsselt versandt werden können.
Entstanden ist es im Rahmen einer Bonusaufgabe der Universität Augsburg im Kurs "Informatik 1" (WiSe24/25).
Es werden keine Garantien für die tatsächliche Sicherheit der Kommunikation übernommen.

*EN*: SEChat is a chat-room cli-tool that is able to send ecnrypted messages over the internet.
This is the result of a semester exercise of the Ausgburg University of the course "Informatik 1" (CS 1) in the winter semester of 2024 to 2025.
There are no guarantees about the actual security of messages being sent.
## Compiling
### Compiling & Installing using CMake
*DE*: Die Einfachste Form das Programm zu Kompilieren ist mit dem Tool "CMake".
Dazu müssen nur folgende Befehle im heruntergeladenen Ordner in die Kommandozeile eingegeben werden:

*EN*: The easiest way to compile the program is with cmake. With it you just need to run the following command in the folder of the cloned repository:
```bash
cmake . && make install
```
### Compiling manually with gcc
```bash
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/main.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/util.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/interface.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/encrypt.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/packet.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/protocol.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/netio.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/net.c
```
**UNIX:**
```bash
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/unix/terminal.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/unix/socket.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -o ./sechat main.o util.o interface.o encrypt.o packet.o protocol.o netio.o net.o terminal.o socket.o
```
**WINDOWS:**
```bash
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/windows/terminal.c
gcc -ansi -lm -Wall -Wextra -Wpedantic -O3 -I./src/ -c src/windows/socket.c
gcc -ansi -lm -lwsock32 -lws2_32 -Wall -Wextra -Wpedantic -O3 -o ./sechat.exe main.o util.o interface.o encrypt.o packet.o protocol.o netio.o net.o terminal.o socket.o
```
## How to use
*DE:* Um sechat zu benutzten kann man einfach ``sechat`` in die Kommandozeile eingeben. Optional kann auch ``sechat connect`` oder ``sechat serve``.
Diese sind gleichbedeutend mit der sofortigen Ausführung der Befehle ``!connect`` und ``!serve`` direkt nach Programmstart. Weiteres dazu unter *Befehle*.

*EN*: To use sechat just type ``sechat`` on the command line and press enter.
You can optionally add ``connect`` or ``serve`` as arguments, which do exactly the same as the commands ``!connect`` and ``!serve`` as you start the program.
### Basic use
*DE:* Unten am Screen befindet sich ein Texteingabefeld. Darin können sowohl Nachrichten gesendet werden, als auch Befehle (beginnend mit ``!``) eingegeben werden.

*EN:* At the bottom of the screen there is a text field from which you can send messages as well as execute commands (which alsways begin with ``!``).

### Befehle / Commands

|Command|Usage|Description Deutsch|Description English|
|:-|:-|:-|:-|
|``!help``| ``!help [cmds...]``|||
|``!quit``| ``!quit``|||
|``!connect``| ``!connect [ip=127.0.0.1] [port=10001]``| Verbindet sich mit Chatraum an der IP ``ip`` am Port ``port``| Connect to chat on ``ip`` and ``port``|
|``!serve``| ``!serve [port=10001]`` |Erstelle Chatraum an ``port``|Create chat room on ``port``|
|``!key``|``!key [name=(you)] [encrypt=] [key=(currently stored key)]``|Überschreibe oder hole ``key`` der Verschlüsselungsmethode &#10; ``encrypt`` der Person ``name``|Set or get ``key`` of encryption &#10;method ``encrypt`` of person ``name``|
|``!encrypt``|``!encrypt [encrypt=]``| Setze eigene Verschlüsselungsmethode auf ``encrypt``|Set current encryption method to ``encrypt``|
|``!decode``|``!decode [enable=on/off]``| (De-)aktiviere automatisches entschlüsseln der Nachrichten |(de-)activate automatic decryption of messages|
|``!name``|``!name [name=]``|Setze den eigenen Namen| Set your own name|
|``!clear``|``!clear``|||
|``!top``|``!top``| Scrolle nach ganz oben| Scroll to the top|
|``!bottom``|``!bottom``|Scrolle nach ganz unten|Scroll to the bottom |
|``!up``|``!up``|Scrolle einen Screen nach oben| Scrolle one page up|
|``!down``|``!down``|Scrolle einen Screen nach unten| Scrolle one page down|

###  Encryption methods

|Name (DE)| Name (EN)| Name in Command|Key format|
|:-|:-|:-|:-|
|Keine Verschlüsselung|No Encryption|``none``|``/``|
|Gartenzaun Transposition| Rail-fence transposition|``rail-fence``|``/``|
|Rot13 Substitution| Rot13 substitution|``rot13``|``/``|
|Rot47 Substitution| Rot47 substitution|``rot47``|``/``|
|Atbash Substitution| Atbash substitution|``atbash``|``/``|
|Caesar Chiffre|Caesar Chiffre|``caesar``| ``0-26`` |
|Vigenere Chiffre|Vigenere Chiffre|``vigenere``| ``any word or text (no spaces)`` |
|Paarweise Vertauschung|Pairwise Substitution|``pair-substitution``|``a substitution table like badcfehgjilknmporqtsvuxwzy where each character is swapped with another in the alphabet``|
|Monoalphabetische Substitution|substitution|``substitution``| ``an alphabet like abdyefghijklmnopqrstuvwxcz where each character appears exactly once``|
|Enigma Rotor| a rotor of the enigma|``enigma-rotor``| ``one of: A, B, C, I, II, III, IV, V, VI, VII, VIII``|
|Enigma|Enigma|``enigma``| ``Semicolon ';' separated list of: Rotor Left, Rotor Middle, Rotor Right, Rotor Reflector, three characters indicating rotor start positions and optionally a pairwise substitution key for the patchpanel``
