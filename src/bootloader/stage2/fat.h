#pragma once
#include "disk.h"
#include "stdint.h"

#pragma pack(push, 1)
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
} FAT_DirectoryEntry;
#pragma pack(pop)

typedef struct {
  int Handle;
  bool IsDirectory;
  uint32_t Position;
  uint32_t Size;
} FAT_File;

enum FAT_Attributes {
  FAT_ATTRIBUTE_READ_ONLY = 0x01,
  FAT_ATTRIBUTE_HIDDEN = 0x02,
  FAT_ATTRIBUTE_SYSTEM = 0x04,
  FAT_ATTRIBUTE_VOLUME_ID = 0x08,
  FAT_ATTRIBUTE_DIRECTORY = 0x10,
  FAT_ATTRIBUTE_ARCHIVE = 0x20,
  FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN |
                      FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

// Sets up data of Bootsector, FAT, and Rootdirecotry in memory starting at
// 0x00000500
bool FAT_Initialize(DISK *disk);

// Gets the current directory/file from the whole path
// Calls FAT_FindFile and gets the dir-entry of the file/directory.
// Calls FAT_OpenEntry to create a new file handle for the dir-entry.
// Closes the file handle if FAT_FindFile does not find the dir-entry.
FAT_File far *FAT_Open(DISK *disk, const char *path);

// Converts the file name to FAT filename format.
// Iterates over dir entries using FAT_ReadEntry() and while loop.
// Compares the entry name with *name and copies the dir-entry data to
// *entryOut.
bool FAT_FindFile(DISK *disk, FAT_File far *file, const char *name,
                  FAT_DirectoryEntry *entryOut);

// Copies a directory entry to *dirEntry pointer
bool FAT_ReadEntry(DISK *disk, FAT_File far *file,
                   FAT_DirectoryEntry *dirEntry);

// Used for reading Bytes of FAT file or direcotry entry  from buffer[] array in
// a safe way.
// Copies the contents from Buffer[] to *dataOut pointer. If buffer dosent
// contain all the data then reads 512 bytes of data from disk to Buffer before
// copying again. Repeats this till all data is copied to *dataOut.
uint32_t FAT_Read(DISK *disk, FAT_File far *file, uint32_t byteCount,
                  uint32_t offsetPosition, void far *dataOut);

// Called to close a file handle
// file file handle is root-dir then resets its position to 0.
// else just sets the opened status to false.
void FAT_Close(FAT_File far *file);

// Uses the dir-entry data from FAT_FindFile() to setup FAT_FileData handle.
// Sets up fields such as: size, IsDirectory, FirstCluster, CurrentCluster.
// And reads first 512 bytes of data from data-section to Buffer[] of the
// handle.
FAT_File far *FAT_OpenEntry(DISK *disk, FAT_DirectoryEntry *entry);
