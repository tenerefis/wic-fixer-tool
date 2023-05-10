# wic-fixer-tool
```
Dependencies:
WiCFix.sln - Visual Studio 2019
```

This tool doubles as an installer and fixes some issues with World in Conflict.  
WiCFix makes it easier for users to apply the patched client dlls from
[WIC-Client](https://github.com/tenerefis/WIC-Client/)

WiCFix downloaded from [Massgate](https://www.massgate.org/) is packaged with official maps that are not part of the main World in Conflict installation  

**Extras:**
- easily change a CD key with the tool
- creates a wicautoexec.txt in the users My Documents\World in Conflict folder
- fixes a registry entry that causes WicEd and WiC Mod SDK installation to fail

**Other requirements:**
1. create a folder named 'data' in the working directory used to run WiCFix.exe
2. copy all the patched dlls, game exes and official maps to the folder
3. copy the original dbghelp.dll to the folder, rename it to dbghelp_old.dll
4. create a file named maps.txt, add each map name, one per line
5. create a wicautoexec.txt with camerafreedom and cameramaxheight commands

**Screenshot::**
![Screenshot](https://www.massgate.org/wicfix/screenshot.png)
