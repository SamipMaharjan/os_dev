//
// All this is to understand how reading disk is don e in fat12 FS.
//

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
typedef uint8_t boolean;
#define true 1
#define false 0

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

  // ... we don't care about code ...

} __attribute__((packed)) BootSector;

typedef struct {
  uint8_t Name[11];
  uint8_t Attributes;
  uint8_t _Reserved;
  uint8_t CreatedTimeTenths;
  uint16_t CreatedTime;
  uint16_t CreatedDate;
  uint16_t AccessedDate;
  uint16_t FirstClusterHigh;
  uint16_t ModifiedTime;
  uint16_t ModifiedDate;
  uint16_t FirstClusterLow; // starts from this cluster.
  uint32_t Size;
} __attribute__((packed)) DirectoryEntry;

BootSector g_BootSector;
uint8_t *g_Fat = NULL;
DirectoryEntry *g_RootDirectory = NULL;
uint32_t g_RootDirectoryEnd;

boolean readBootSector(FILE *disk) {
  // copying the raw bytes in the first sector to g_BootSector pointer.
  return fread(&g_BootSector, sizeof(g_BootSector), 1, disk) > 0;
}

boolean readSectors(FILE *disk, uint32_t lba, uint32_t count, void *bufferOut) {
  boolean ok = true;

  // sets the file cursor in the right place before reading.
  ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);

  // reads the file and outputs it into bufferOut pointer.
  ok = ok &&
       (fread(bufferOut, g_BootSector.BytesPerSector, count, disk) == count);
  return ok;
}

boolean readFat(FILE *disk) {
  // Allocating memory for 1 FAT.
  g_Fat = (uint8_t *)malloc(g_BootSector.SectorsPerFat *
                            g_BootSector.BytesPerSector); // 2 * 512

  // Reading the Sectors containg FAT and outputing them in g_Fat.
  return readSectors(disk, g_BootSector.ReservedSectors,
                     g_BootSector.SectorsPerFat, g_Fat);
}

boolean readRootDirectory(FILE *disk) {
  // The the lba of root directory
  uint32_t lba = g_BootSector.ReservedSectors +
                 g_BootSector.SectorsPerFat * g_BootSector.FatCount;

  // size of root directory in bytes
  uint32_t size = sizeof(DirectoryEntry) * g_BootSector.DirEntryCount;

  // no of sectors used by root_directory
  uint32_t sectors = (size / g_BootSector.BytesPerSector);

  // if it spills into next sector then allocate the next sector for it as well.
  if (size % g_BootSector.BytesPerSector > 0)
    sectors++;

  // assigns the sector where contents of root directory ends.
  g_RootDirectoryEnd = lba + sectors;

  // allocating memory for rootDirecotry.
  g_RootDirectory =
      (DirectoryEntry *)malloc(sectors * g_BootSector.BytesPerSector);

  // reads the content of root_directory to g_RootDirectory
  return readSectors(disk, lba, sectors, g_RootDirectory);
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
