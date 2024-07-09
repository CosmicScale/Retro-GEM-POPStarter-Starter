# Retro-GEM-POPStarter-Starter

## Overview

Originally designed with PS1 games and POPStarter in mind, Retro-GEM-POPStarter-Starter is a simply PS2 app for use with HDDOSD or PSBBN that launches any `.elf` file found in the current working directory and sets the Retro GEM Game ID accordingly.

## Setting up POPStarter

Follow this guide to set up POPStarter and get PS1 games running on your PS2 HDD: [POPStarter Quickstart Guide](https://bitbucket.org/ShaolinAssassin/popstarter-documentation-stuff/wiki/quickstart-hdd).

All games should be titled in the standard format used by OPL, with the Game ID as a prefix, for example, `SCUS_941.94.Gran Turismo.VCD`, and placed in the `__.POPS` partition.

## Setting up POPStarter-Starter

1. Create a partition on your PS2 HDD with the prefix `PP.` followed by a name of your choosing.

2. Put the signed executable `POPStarter-Starter.kelf` into the root of the above partition.

3. Place the `POPSTARTER.ELF` file into the root of the same partition, making sure to rename it to match the `.VCD` file you wish to launch while keeping the `.elf` extension, for example, `SCUS_941.94.Gran Turismo.elf`.

4. Create the text file `system.cnf`. `BOOT2` should be set to `pfs/:POPStarter-Starter.kelf`:

    ```ini
    BOOT2 = pfs:/POPStarter-Starter.kelf
    VER = 1.00
    VMODE = NTSC
    HDDUNITPOWER = NICHDD
    ```

5. Source a standard PS2 game icon or make your own. Rename it to `list.ico`.

6. Create the text file `icon.sys`. For example:

    ```ini
    PS2X
    title0=Gran Turismo
    title1=SCUS-94194
    bgcola=0
    bgcol0=0,0,0
    bgcol1=0,0,0
    bgcol2=0,0,0
    bgcol3=0,0,0
    lightdir0=1.0,-1.0,1.0
    lightdir1=-1.0,1.0,-1.0
    lightdir2=0.0,0.0,0.0
    lightcolamb=64,64,64
    lightcol0=64,64,64
    lightcol1=16,16,16
    lightcol2=0,0,0
    uninstallmes0=
    uninstallmes1=
    uninstallmes2=
    ```

7. Inject `system.cnf`, `list.ico`, and `icon.sys` into the header of your partition using [HDL Dump](https://github.com/ps2homebrew/hdl-dump). For example:

    ```cmd
    hdl_dump modify_header hdd<Disk Number>: <PP.PartitionName>
    ```

## For PSBBN only

8. Create the folder `res` in `PP.PartitionName`. This folder will contain all resources.

9. Inside the `res` folder, place the text file `info.sys`. Example file:

    ```ini
    title = Gran Turismo
    title_id = SCUS-94194
    title_sub_id = 0
    release_date =
    developer_id =
    publisher_id =
    note =
    content_web =
    image_topviewflag = 0
    image_type = 0
    image_count = 1
    image_viewsec = 600
    copyright_viewflag = 0
    copyright_imgcount = 0
    genre =
    parental_lock = 1
    effective_date = 0
    expire_date = 0
    area = J
    violence_flag = 0
    content_type = 255
    content_subtype = 0
    ```

10. Place `jkt_001.png` in the `res` folder. This picture will be used as the thumbnail for the game. It can be the same picture as used in OPL.

# Notes

This not only works for PS1 games with POPStarter, you can place any `.elf` file in the partition you created and it will be launched. The Retro GEM Game ID will be set to the first 11 characters of the `.elf` file's name.

If you compile this program from source, the resulting file will be `POPStarter-Starter.elf`. You will need to sign the executable before it can be used.

Sign the executable with [this application](https://www.psx-place.com/resources/kelftool-fmcb-compatible-fork.1104/):

    ```cmd
    kelftool encrypt mbr POPStarter-Starter.elf POPStarter-Starter.kelf
    ```

If you have downloaded `POPStarter-Starter.kelf` from `Releases`, then the file is already signed so this step is not necessary.

## Credits

Written by [CosmicScale](https://github.com/CosmicScale)  
Inspired by [OPL-Launcher](https://github.com/ps2homebrew/OPL-Launcher)  
Uses GameID code based on the [AppCakeLtd](https://github.com/AppCakeLtd) [gameid-with-gem](https://github.com/AppCakeLtd/Open-PS2-Loader/tree/gameid-with-gem) fork of Open-PS2-Loader.