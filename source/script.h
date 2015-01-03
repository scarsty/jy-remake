#ifndef JY_SCRIPT_H
#define JY_SCRIPT_H

int		Script_Init(void);
void	Script_Quit(void);
int		Script_CallFunction(const char *filename);
int		Script_GetInteger(const char *name);
void	Script_GetString(const char *name, char *str);
int		Script_LoadAndRun(const char *filename);
SDL_bool Script_GetBoolean(const char *name);
void	Script_GetGlobal(const char *name);

#endif // JY_SCRIPT_H

