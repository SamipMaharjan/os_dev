//
// All this is to understand how reading disk is don e in fat12 FS.
//

#include "fat.h"
#include "disk.h"
#include "memdefs.h"
#include "stdint.h"
#include "stdio.h"
#include "utility.h"

typedef uint8_t boolean;
#define true 1
#define false 0

#define SECTOR_SIZE 512
#define MAX_FILE_HANDLES 10
#define MAX_PATH_SIZE 256
#define ROOT_DIRECTORY_HANDLE -1

#pragma pack(push, 1)
typedef struct {
  uint8_t BootJumpInstruction[3];
  uint8_t OemIdentifier[8];
  uint16_t BytesPerSector;   // No of bytes allocated for a sector normally 512
  uint8_t SectorsPerCluster; // No of sectors per cluster.
  uint16_t ReservedSectors; // Number of sectors before the first FAT, including
                            // the boot sector. so is RS = 1 then FAT starts at
                            // sector 2
  uint8_t FatCount;         // No of FAT tables.
  uint16_t DirEntryCount;   // No of items that are allowed in a directory.
  uint16_t TotalSectors;
  uint8_t MediaDescriptorType;
  uint16_t
      SectorsPerFat; // No of sectors allocated for a FAT. Typicall 2 in FAT 12
  uint16_t SectorsPerTrack;
  uint16_t Heads;
  uint32_t HiddenSectors;
  uint32_t LargeSectorCount;

  // extended boot record
  uint8_t DriveNumber;
  uint8_t _Reserved;
  uint8_t Signature;
  uint32_t VolumeId;       // serial number, value doesn't matter
  uint8_t VolumeLabel[11]; // 11 bytes, padded with spaces
  uint8_t SystemId[8];

} FAT_BootSector;
#pragma pack(pop)

typedef struct {
  // will contain data that will be returned to the user thus named public
  FAT_File Public;
  bool Opened;                     // If file slot is available or not.
  uint32_t FirstCluster;           // first cluster where the data of file is.
  uint32_t CurrentCluster;         // current cluster that is being read.
  uint32_t CurrentSectorInCluster; //  current sector in that cluster.
  uint8_t Buffer[SECTOR_SIZE];     // Cache for file data, minimize reads.
} FAT_FileData;

typedef struct {

  union {
    FAT_BootSector BootSector;
    uint8_t BootSectorBytes[SECTOR_SIZE];
  } BS;

  FAT_FileData RootDirectory;

  FAT_FileData OpenedFiles[MAX_FILE_HANDLES];
} FAT_Data;

static FAT_Data far *g_Data;

FAT_BootSector g_BootSector;
static uint8_t far *g_Fat = NULL;
// static FAT_DirectoryEntry *g_RootDirectory = NULL;
static uint32_t g_DataSectionLba;

boolean FAT_ReadFat(DISK *disk);
boolean FAT_ReadRootDirectory(DISK *disk);

boolean FAT_ReadBootSector(DISK *disk) {
  // copying the raw bytes in the first sector to g_BootSector pointer.
  // return fread(&g_BootSector, sizeof(g_BootSector), 1, disk) > 0;
  return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

// Reads: boot sector, FAT, and root directory contents to 0x5000
bool FAT_Initialize(DISK *disk) {
  g_Data = (FAT_Data far *)
      MEMORY_FAT_ADDR; // far pointer pointing to instance of FAT_DATA type
                       // This address will contain.

  // Read boot sector to g_Data->BS.BootSectorBytes
  if (!FAT_ReadBootSector(disk)) {
    printf("read bootsector failed \r\n");
    return false;
  }

  // READ FAT
  //
  // Put the g_Fat pointer right after g_Data pointer.
  // Have to be efficient with 1MB memory.
  g_Fat = (uint8_t far *)g_Data + sizeof(FAT_Data);
  uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector *
                     g_Data->BS.BootSector.SectorsPerFat;
  if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
    printf(
        "FAT: Not enough memory to read FAT! Required %lu, only have %u \r\n",
        sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
    return false;
  }

  if (!FAT_ReadFat(disk)) {
    printf("FAT: read FAT failed \r\n", sizeof(FAT_Data) + fatSize,
           MEMORY_FAT_SIZE);
    return false;
  }

  // read root directory
  //
  // Put g_RootDirectory pointer after end of g_Fat pointer.
  // g_RootDirectory = (FAT_DirectoryEntry far *)(g_Fat + fatSize);

  uint32_t rootDirLba =
      g_Data->BS.BootSector.ReservedSectors +
      g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;

  // Calculate rootDirSize
  uint32_t rootDirSize =
      sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;

  // Make sure rootDirSize is divisible by 512 / sector size.
  rootDirSize = align(rootDirSize, g_Data->BS.BootSector.BytesPerSector);

  // if (sizeof(FAT_Data) + fatSize + rootDirSize >= MEMORY_FAT_SIZE) {
  //   printf(
  //       "FAT: Not enough memory to read FAT! Required %lu, only have %u
  //       \r\n", sizeof(FAT_Data) + fatSize + rootDirSize);
  //   return false;
  // }

  // if (!FAT_ReadRootDirectory(disk)) {
  //   printf("FAT: read FAT failed \r\n", sizeof(FAT_Data) + fatSize,
  //          MEMORY_FAT_SIZE);
  //   return false;
  // }

  // Open root directory
  g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
  g_Data->RootDirectory.Public.isDirectory = true;
  g_Data->RootDirectory.Public.position = 0;
  g_Data->RootDirectory.Public.Size =
      sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
  g_Data->RootDirectory.Opened = true;
  g_Data->RootDirectory.FirstCluster = 0;
  g_Data->RootDirectory.CurrentCluster = 0;
  g_Data->RootDirectory.CurrentSectorInCluster = 0;

  if (!DISK_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
    printf("FAT: Read root directory filed");
    return false;
  }

  // clacualte data section??????????
  uint32_t rootDirSectors =
      (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) /
      g_Data->BS.BootSector.BytesPerSector;
  g_DataSectionLba = rootDirLba + rootDirSectors;

  // reset opened files
  for (int i = 0; i < MAX_FILE_HANDLES; i++) {
    g_Data->OpenedFiles[i].Opened = false;
  }
  return true;
}

FAT_File far *FAT_OpenEntry(DISK *disk, FAT_DirectoryEntry *entry)

    // boolean readSectors(FILE *disk, uint32_t lba, uint32_t count, void
    // *bufferOut) {
    //   boolean ok = true;
    //
    //   // sets the file cursor in the right place before reading.
    //   ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) ==
    //   0);
    //
    //   // reads the file and outputs it into bufferOut pointer.
    //   ok = ok &&
    //        (fread(bufferOut, g_BootSector.BytesPerSector, count, disk) ==
    //        count);
    //   return ok;
    // }

    boolean FAT_ReadFat(DISK *disk) {

  return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors,
                          g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

// returns the fileEntry when the filename matches
DirectoryEntry *findFile(const char *name) {
  for (uint32_t i = 0; i < g_BootSector.DirEntryCount; i++) {
    if (memcmp(name, g_RootDirectory[i].Name, 11) == 0)
      return &g_RootDirectory[i];
  }

  return NULL;
}

boolean readFile(DirectoryEntry *fileEntry, FILE *disk, uint8_t *outputBuffer) {
  boolean ok = true;
  uint16_t currentCluster = fileEntry->FirstClusterLow;

  do {
    uint32_t lba = g_RootDirectoryEnd +
                   (currentCluster - 2) * g_BootSector.SectorsPerCluster;
    ok = ok &&
         readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
    outputBuffer +=
        g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

    // Since each entry in fat 12 is 12 bits and there is no padding between
    // entries. 3 bytes will contain 2 entries. Each byte has a address. But
    // now that half fat entries are in middle of a byte (of 3 bytes). We apply
    // the formula ( N * 3 ) / 2 to:
    // 1. goto the entry if the index of the entry is even number.
    // ------------------------OR--------------------------
    // 2. goto the address that contains half of two entries if the entry is odd
    // number.
    //
    // Then depending on the type of entry we apply the below if
    // conditions to get the full entry.
    uint32_t fatIndex = currentCluster * 3 / 2;
    if (currentCluster % 2 == 0)
      // the even entry. So only taking the lower 12 bits and setting upper 4
      // bits to 0. Using masking.
      currentCluster = (*(uint16_t *)(g_Fat + fatIndex)) & 0x0FFF;
    else
      //
      // >> (right shift) : moves bits to least significant bits
      // << (left shift): moves bits to most significant bits
      currentCluster = (*(uint16_t *)(g_Fat + fatIndex)) >> 4;

    // 0x0ff8 - 0x0fff mean end of cluster chain
  } while (ok && currentCluster < 0x0FF8);

  return ok;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Syntax: %s <disk image> <file name>\n", argv[0]);
    return -1;
  }

  FILE *disk = fopen(argv[1], "rb");
  if (!disk) {
    fprintf(stderr, "Cannot open disk image %s!\n", argv[1]);
    return -1;
  }

  // Just read the boot sector and see if it exists or not.
  // Populates g_BootSector
  if (!readBootSector(disk)) {
    fprintf(stderr, "Could not read boot sector!\n");
    return -2;
  }

  // populates g_Fat variable
  if (!readFat(disk)) {
    fprintf(stderr, "Could not read FAT!\n");
    free(g_Fat);
    return -3;
  }

  // populates the g_RootDirectory variable
  if (!readRootDirectory(disk)) {
    fprintf(stderr, "Could not read FAT!\n");
    free(g_Fat);
    free(g_RootDirectory);
    return -4;
  }

  // searches and returns the file entry in root dir
  DirectoryEntry *fileEntry = findFile(argv[2]);
  if (!fileEntry) {
    fprintf(stderr, "Could not find file %s!\n", argv[2]);
    free(g_Fat);
    free(g_RootDirectory);
    return -5;
  }

  // tries to read file.
  uint8_t *buffer =
      (uint8_t *)malloc(fileEntry->Size + g_BootSector.BytesPerSector);
  if (!readFile(fileEntry, disk, buffer)) {
    fprintf(stderr, "Could not read file %s!\n", argv[2]);
    free(g_Fat);
    free(g_RootDirectory);
    free(buffer);
    return -5;
  }

  for (size_t i = 0; i < fileEntry->Size; i++) {
    if (isprint(buffer[i]))
      fputc(buffer[i], stdout);
    else
      printf("<%02x>", buffer[i]);
  }
  printf("\n");

  free(buffer);
  free(g_Fat);
  free(g_RootDirectory);
  return 0;
}
//
// boolean FAT_ReadRootDirectory(DISK *disk) {
//   // The the lba of root directory
//   uint32_t lba =
//       g_Data->BS.BootSector.ReservedSectors +
//       g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
//
//   // size of root directory in bytes
//   uint32_t size =
//       sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
//
//   // no of sectors used by root_directory
//   uint32_t sectors = (size + g_Data->BS.BootSector.BytesPerSector - 1) /
//                      g_Data->BS.BootSector.BytesPerSector;
//
//   g_RootDirectoryEnd = lba + sectors;
//
//   // reads the content of root_directory to g_RootDirectory
//   return DISK_ReadSectors(disk, lba, sectors, g_RootDirectory);
// }
