// fat.c
//  All this is to understand how reading disk is don e in fat12 FS.
//

#include "fat.h"
#include "ctype.h"
#include "disk.h"
#include "memdefs.h"
#include "memory.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "utility.h"

typedef uint8_t boolean;
#define true 1
#define false 0
#define SECTOR_SIZE 512
#define MAX_FILE_HANDLES 10
#define MAX_PATH_SIZE 256
#define ROOT_DIRECTORY_HANDLE -1

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

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
uint32_t FAT_ClusterToLba(uint32_t cluster);

uint32_t FAT_NextCluster(uint32_t currentCluster) {
  uint32_t fatIndex = currentCluster * 3 / 2;
  if (currentCluster % 2 == 0) {
    return (*(uint16_t far *)(g_Fat + fatIndex)) & 0x0FFF;
  } else {
    return (*(uint16_t far *)(g_Fat + fatIndex)) >> 4;
  }
}
boolean FAT_ReadBootSector(DISK *disk) {
  // copying the raw bytes in the first sector to g_BootSector pointer.
  // return fread(&g_BootSector, sizeof(g_BootSector), 1, disk) > 0;

  return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

uint32_t FAT_Read(DISK *disk, FAT_File far *file, uint32_t byteCount,
                  void far *dataOut) {

  // get file data using handle
  FAT_FileData far *fd = (file->Handle == ROOT_DIRECTORY_HANDLE)
                             ? &g_Data->RootDirectory
                             : &g_Data->OpenedFiles[file->Handle];

  uint8_t *u8DataOut = (uint8_t *)dataOut;

  // If its a direcotry then allow reading past the file for the current sector
  // As subdirectory size is 0.
  if (!fd->Public.IsDirectory) {
    // Total size left to be read
    // Also prevents reading past a file.
    // if byteCount exceedes the file size then it wont read past the file.
    byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);
  }

  while (byteCount > 0) {
    // no of data left to read in buffer
    // so if position = 0; leftInBuffer = 512;
    // and if position = 1; leftInBuffer = 511;
    uint32_t leftInBuffer = (SECTOR_SIZE - fd->Public.Position % SECTOR_SIZE);

    // if byteCount exceedes the buffer size then it wont read past the current
    // 512 sized buffer
    uint32_t take = min(byteCount, leftInBuffer);

    // copy 1 sector or less of data from fd->Buffer to u8DataOut
    memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
    u8DataOut += take;
    fd->Public.Position += take;
    byteCount -= take;

    // If we need to read more data
    if (byteCount > 0) {
      // Special handling for root directory as FAT_Read is used to read root
      // directory entry too
      if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE) {
        // Increment LBA
        ++fd->CurrentCluster;
        // read the next 512 bytes of root directory to buffer
        if (!DISK_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer)) {
          printf("FAT: read error!\r\n");
          break;
        }
      } else {
        // calculate next cluster & sector to read
        if (++fd->CurrentSectorInCluster >=
            g_Data->BS.BootSector.SectorsPerCluster) {
          fd->CurrentSectorInCluster = 0;
          fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
        }

        // should never happen
        // byteCount should not be greater than zero if byteCount is not greater
        // than remaining size of file or bytes left in buffer
        if (fd->CurrentCluster >= 0xFF8) {
          printf("FAT: read error! invalid next cluster!\r\n");
          break;
        }

        // read next sector
        if (!DISK_ReadSectors(disk,
                              FAT_ClusterToLba(fd->CurrentCluster) +
                                  fd->CurrentSectorInCluster,
                              1, fd->Buffer)) {
          printf("FAT: read error!\r\n");
          break;
        }
      }
    }
  }
  return u8DataOut - (uint8_t *)dataOut;
}

bool FAT_ReadEntry(DISK *disk, FAT_File far *file,
                   FAT_DirectoryEntry *dirEntry) {
  bool returnValue = FAT_Read(disk, file, sizeof(FAT_DirectoryEntry),
                              dirEntry) == sizeof(FAT_DirectoryEntry);
  return returnValue;
}

void FAT_Close(FAT_File far *file) {
  // if its root_directory then reset position to 0
  if (file->Handle == ROOT_DIRECTORY_HANDLE) {
    file->Position = 0;
    g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
  } else {
    // if its normal file handle then close it.
    g_Data->OpenedFiles[file->Handle].Opened = false;
  }
}

// Reads: boot sector, FAT, and root directory contents to 0x5000
// Initializes g_Data, which contains bootsector and RootDirectory, and fat
// table into memory
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
  // Have to be efficient with 0.5MB memory.
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
  uint32_t rootDirLba =
      g_Data->BS.BootSector.ReservedSectors +
      g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;

  uint32_t rootDirSize =
      sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;

  // Make sure rootDirSize is divisible by 512 / sector size.
  rootDirSize = align(rootDirSize, g_Data->BS.BootSector.BytesPerSector);

  // Open root directory
  g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
  g_Data->RootDirectory.Public.IsDirectory = true;
  g_Data->RootDirectory.Public.Position = 0;
  g_Data->RootDirectory.Public.Size =
      sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
  g_Data->RootDirectory.Opened = true;
  // FirstCluster and CurrentCluster are set to LBA only because its root
  // directory and will not be equal to LBA for sub-directory entries
  g_Data->RootDirectory.FirstCluster = rootDirLba;
  g_Data->RootDirectory.CurrentCluster = rootDirLba;
  g_Data->RootDirectory.CurrentSectorInCluster = 0;

  if (!DISK_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
    printf("FAT: Read root directory filed");
    return false;
  }

  // calculate data section
  uint32_t rootDirSectors =
      (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) /
      g_Data->BS.BootSector.BytesPerSector;
  g_DataSectionLba = rootDirLba + rootDirSectors;

  // reset opened files handles
  for (int i = 0; i < MAX_FILE_HANDLES; i++) {
    g_Data->OpenedFiles[i].Opened = false;
  }

  return true;
}

FAT_File far *FAT_OpenEntry(DISK *disk, FAT_DirectoryEntry *entry) {
  int handle = -1;

  // find empty handle
  for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
    if (!g_Data->OpenedFiles[i].Opened)
      handle = i;
  }

  // if out of handles.
  if (handle < 0) {
    printf("FAT:out of file handles");
    return false;
  }

  // Setup fields for the Opened Entry
  FAT_FileData far *fd = &g_Data->OpenedFiles[handle];
  fd->Public.Handle = handle;
  fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
  fd->Public.Position = 0;
  fd->Public.Size = entry->Size;
  fd->FirstCluster =
      entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
  fd->CurrentCluster = fd->FirstCluster;
  fd->CurrentSectorInCluster = 0;

  if (!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1,
                        fd->Buffer)) {
    printf("FAT:read error \r\n");
    return false;
  }

  fd->Opened = true;

  return &fd->Public;
}

uint32_t FAT_ClusterToLba(uint32_t cluster) {
  return g_DataSectionLba +
         (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

bool FAT_FindFile(DISK *disk, FAT_File far *file, const char *name,
                  FAT_DirectoryEntry *entryOut) {
  char fatName[11];
  FAT_DirectoryEntry entry;

  // convert from name to fat name
  memset(fatName, ' ', sizeof(fatName));
  const char *ext = strchr(name, '.');
  if (ext == NULL)
    ext = name + 11;

  for (int i = 0; i < 8 && name + i < ext; i++) {
    fatName[i] = toupper(name[i]);
    if (name[i] == 0)
      fatName[i] = ' ';
  }

  if (ext != NULL) {
    for (int i = 0; i < 3 && ext[i]; i++) {
      fatName[i + 8] = toupper(ext[i + 1]);
      if (name[i] == 0)
        fatName[i] = ' ';
    }
  }

  // uses FAT_ReadEntry to get the direcotry entry to *entry one by one
  while (FAT_ReadEntry(disk, file, &entry)) {
    char *temp = "TEST    TXT";
    // compares the filename with current entry name.
    if (memcmp(fatName, entry.Name, 11) == 0) {
      *entryOut = entry;
      return true;
    }
  }

  return false;
}

FAT_File far *FAT_Open(DISK *disk, const char *path) {
  char name[MAX_PATH_SIZE];

  // ignore leading slash
  if (path[0] == '/')
    path++;

  // FAT_File far *parent = NULL;
  FAT_File far *current = &g_Data->RootDirectory.Public;

  while (*path != '\0') {
    // extract next file name from path
    bool isLast = false;
    const char *delim = strchr(path, '/');

    if (delim != NULL) {
      memcpy(name, path, delim - path);
      name[delim - path + 1] = '\0';
      path = delim + 1;
    } else {
      unsigned len = strlen(path);
      memcpy(name, path, len);
      name[len + 1] = '\0';
      path += len;
      isLast = true;
    }

    FAT_DirectoryEntry entry;

    // find directory entry in the current directory
    if (FAT_FindFile(disk, current, name, &entry)) {
      // check if its a directory
      // cecks if a file is put in the midlle of the path string. Like
      // /dir/file/file.
      if (!isLast && (entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == 0) {
        printf("FAT: %s is not a directory. \r\n", name);
        return NULL;
      }
      // open the directory entry searched using FAT_FindFile()
      // and put it in the file handle

      current = FAT_OpenEntry(disk, &entry);

    } else {
      FAT_Close(current);

      printf("FAT: %s not found \r\n", name);
      return NULL;
    }
  }
  return current;
}
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
//
// printf("BEFORE CONVERSION\r\n");
// for (int i = 0; i < 11; i++) {
//   if (name[i] == ' ') {
//     // puts("*");
//     printf(" %x ", name[i]);
//   } else {
//     // putc(name[i]);
//     printf(" %x ", name[i]);
//   }
// }
// printf("hh\r\n");
