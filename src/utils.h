#ifndef LASM2_UTILS_H
#define LASM2_UTILS_H

#define NEW(_type, _size) (_type*)calloc((_size), sizeof(_type))
int str_chr_count(const char* str, char chr);
char* str_first_chr(const char* str, char chr);

#ifdef LASM2_UTILS_IMPLEMENTATION

int str_chr_count(const char* str, char chr){
  // Source - https://stackoverflow.com/a/4235538
  // Posted by Fabian Giesen
  // Retrieved 2026-06-20, License - CC BY-SA 2.5
  int count = 0;
  for(int i = 0; str[i]; i++)
    count += (str[i] == chr);
  return count;
}
char* str_first_chr(const char* str, char chr){
  for(int i = 0; str[i]; i++)
    if(str[i] == chr) return str+i;
  return str;
}

#endif // LASM2_UTILS_IMPLEMENTATION

#endif // LASM2_UTILS_H