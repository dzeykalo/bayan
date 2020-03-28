bayan - duplicate search utility
options:
  -h [ --help ]             show help
  -i [ --input ] arg        name input files or directory
  -b [ --block-size ] arg   block size in bytes. Default 5
  -s [ --hash ] arg         Hash algorithm: sha1 or crc32. Default crc32
example:
  bayan -i directory/file -i file       ->  searching for duplicate files among all files in folder and file. Default Options
  bayan -i file -i file -b 5 -s sha1    ->  searching for duplicate files, where block size = 5, hash algorithm = sha1