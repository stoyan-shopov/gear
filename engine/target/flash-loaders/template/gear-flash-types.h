

struct gear_flash_data
{
	void	* scratch_area;
	int	scratch_area_len;
	int	(*flash_init)(void);
	int	(*flash_shutdown)(void);
	int	(*flash_erase)(void * addr, int size);
	int	(*flash_write)(void * addr, void * data, int size);
}

