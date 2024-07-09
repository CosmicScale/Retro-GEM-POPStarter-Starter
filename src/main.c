#include "include/main.h"

// Function to load modules for HDD access
static void LoadModules()
{
    SifExecModuleBuffer(ps2dev9_irx, size_ps2dev9_irx, 0, NULL, NULL);
    SifExecModuleBuffer(iomanx_irx, size_iomanx_irx, 0, NULL, NULL);
    SifExecModuleBuffer(filexio_irx, size_filexio_irx, 0, NULL, NULL);
    fileXioInit();
    SifExecModuleBuffer(ps2atad_irx, size_ps2atad_irx, 0, NULL, NULL);
    SifExecModuleBuffer(ps2hdd_irx, size_ps2hdd_irx, 0, NULL, NULL);
    SifExecModuleBuffer(ps2fs_irx, size_ps2fs_irx, 0, NULL, NULL);
}

// Function to find .elf file
static int FindElfFile(char *elfFile, size_t elfFileSize)
{
    DIR *d;
    struct dirent *dir;
    d = opendir("pfs0:/");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            char *dot = strrchr(dir->d_name, '.');
            if (dot && (strcasecmp(dot, ".elf") == 0)) {
                strncpy(elfFile, dir->d_name, elfFileSize - 1);
                elfFile[elfFileSize - 1] = '\0'; // Ensure null-termination
                closedir(d);
                return 0; // Success
            }
        }
        closedir(d);
    }
    return -1; // Failure
}

// Function to calculate CRC
static uint8_t gameid_calc_crc(const uint8_t *data, int len) {
    uint8_t crc = 0x00;
    for (int i = 0; i < len; i++) {
        crc += data[i];
    }
    return 0x100 - crc;
}

// Function to draw game ID using GSKit
void gameid_draw(GSGLOBAL *gsGlobal, int screenWidth, int screenHeight, const char* game_id) {
    uint8_t data[64] = { 0 };

    int dpos = 0;
    data[dpos++] = 0xA5;  // detect word
    data[dpos++] = 0x00;  // address offset
    dpos++;
    data[dpos++] = strlen(game_id);  // Length of game_id

    memcpy(&data[dpos], game_id, strlen(game_id));
    dpos += strlen(game_id);

    data[dpos++] = 0x00;
    data[dpos++] = 0xD5;  // end word
    data[dpos++] = 0x00;  // padding

    int data_len = dpos;
    data[2] = gameid_calc_crc(&data[3], data_len - 3);

    int xstart = (screenWidth / 2) - (data_len * 8);
    int ystart = screenHeight - (((screenHeight / 8) * 2) + 20);
    int height = 2;

    for (int i = 0; i < data_len; i++) {
        for (int ii = 7; ii >= 0; ii--) {
            int x = xstart + (i * 16 + ((7 - ii) * 2));
            int x1 = x + 1;

            gsKit_prim_sprite(gsGlobal, x, ystart, x1, ystart + height, 1, GS_SETREG_RGBA(0xFF, 0x00, 0xFF, 0x80));

            uint32_t color = (data[i] >> ii) & 1 ? GS_SETREG_RGBA(0x00, 0xFF, 0xFF, 0x80) : GS_SETREG_RGBA(0xFF, 0xFF, 0x00, 0x80);
            gsKit_prim_sprite(gsGlobal, x1, ystart, x1 + 1, ystart + height, 1, color);
        }
    }
}

// Function to handle GSKit initialization and game ID display
void DisplayGameID(const char *elfFile)
{
    // Initialize GSKit
    GSGLOBAL *gsGlobal = gsKit_init_global();

    // Set the screen mode
    gsGlobal->Mode = GS_MODE_DTV_480P;  // Set to 480p mode
    gsGlobal->Width = 640;  // Width of the screen
    gsGlobal->Height = 448;  // Height of the screen
    gsGlobal->Interlace = GS_NONINTERLACED;  // Non-interlaced mode for progressive scan
    gsGlobal->Field = GS_FRAME;  // Frame mode
    gsGlobal->PSM = GS_PSM_CT24;  // Pixel storage format (24-bit color)
    gsGlobal->PSMZ = GS_PSMZ_32;  // Z buffer format (32-bit depth)
    gsGlobal->DoubleBuffering = GS_SETTING_OFF;  // Disable double buffering
    gsGlobal->ZBuffering = GS_SETTING_OFF;  // Disable Z buffering

    // Initialize the screen
    gsKit_vram_clear(gsGlobal);  // Clear VRAM
    gsKit_init_screen(gsGlobal);

    // Set the game ID to the first 11 characters of elfFile
    char game_id[12]; // 11 characters + null terminator
    strncpy(game_id, elfFile, 11);
    game_id[11] = '\0'; // Ensure null-termination

    // Display game ID
    gameid_draw(gsGlobal, gsGlobal->Width, gsGlobal->Height, game_id);

    // Sync and flip the Graphics Synthesizer
    gsKit_sync_flip(gsGlobal);
    gsKit_queue_exec(gsGlobal);
}

// Funtion for returning to OSD when error is encountered
static inline void BootError(void)
{
    SifExitRpc();
    char *args[2] = {"BootError", NULL};
    ExecOSD(1, args);
}

int main(int argc, char *argv[])
{
    char cwd[256];     // Buffer to hold current working directory
    char elfFile[256];

    // Initialize and configure enviroment
    if (argc > 1) {
        while (!SifIopReset(NULL, 0)) {}
        while (!SifIopSync()) {}
        SifInitRpc(0);
    }

    SifInitIopHeap();
    SifLoadFileInit();
    sbv_patch_enable_lmb();
    LoadModules();
    SifLoadFileExit();
    SifExitIopHeap();

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        BootError();
        return 1;
    }

     // Remove ":pfs:" from the end of the current working directory path
    char *pfs_pos = strstr(cwd, ":pfs:");
    if (pfs_pos) {
        *pfs_pos = '\0'; // Terminate the string before ":pfs:"
    }

    // Mount the current working directory
    if (fileXioMount("pfs0:", cwd, FIO_MT_RDWR) != 0) {
        return 1;
    }

    // Find an ELF file in the current working directory
    if (FindElfFile(elfFile, sizeof(elfFile)) != 0) {
        BootError();
        return 1;
    }

    // Initialize GSKit and display game ID
    DisplayGameID(elfFile);

    // Load the found ELF file
    char fullFilePath[512];
    snprintf(fullFilePath, sizeof(fullFilePath), "pfs0:/%s", elfFile);
    if (LoadELFFromFile(fullFilePath, 0, NULL) != 0) {
        BootError();
        return 1;
    }

    return 0;
}
