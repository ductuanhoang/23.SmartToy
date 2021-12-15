/*
 * external_flash.c
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "external_flash.h"
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
static const char *TAG = "exterl_flash";
// Mount path for the partition
const char *base_path = "/extflash";
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define EXTERNAL_FLASH_PIN_NUM_MISO 12
#define EXTERNAL_FLASH_PIN_NUM_MOSI 13
#define EXTERNAL_FLASH_PIN_NUM_CLK 14
#define EXTERNAL_FLASH_PIN_NUM_CS 15
// Handle of the wear levelling library instance
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

static esp_flash_t *init_ext_flash(void);
static void list_data_partitions(void);
static bool mount_fatfs(const char *partition_label);
static void get_fatfs_usage(size_t *out_total_bytes, size_t *out_free_bytes);
static const esp_partition_t *add_partition(esp_flash_t *ext_flash, const char *partition_label);
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
/***********************************************************************************************************************
* Function Name: user_external_flash_init
* Description  : 
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void user_external_flash_init(void)
{
    // Set up SPI bus and initialize the external SPI Flash chip
    esp_flash_t *flash = init_ext_flash();
    if (flash == NULL)
    {
        return;
    }

    // Add the entire external flash chip as a partition
    const char *partition_label = "storage";
    add_partition(flash, partition_label);

    // List the available partitions
    list_data_partitions();

    // Initialize FAT FS in the partition
    if (!mount_fatfs(partition_label))
    {
        return;
    }
}
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
static esp_flash_t *init_ext_flash(void)
{
    const spi_bus_config_t bus_config = {
        .mosi_io_num = EXTERNAL_FLASH_PIN_NUM_MOSI,
        .miso_io_num = EXTERNAL_FLASH_PIN_NUM_MISO,
        .sclk_io_num = EXTERNAL_FLASH_PIN_NUM_CLK,
        .quadhd_io_num = NULL,
        .quadwp_io_num = NULL,
    };

    const esp_flash_spi_device_config_t device_config = {
        .host_id = VSPI_HOST,
        .cs_id = 0,
        .cs_io_num = EXTERNAL_FLASH_PIN_NUM_CS,
        .io_mode = SPI_FLASH_DIO,
        .speed = ESP_FLASH_40MHZ};

    ESP_LOGI(TAG, "Initializing external SPI Flash");
    ESP_LOGI(TAG, "Pin assignments:");
    ESP_LOGI(TAG, "MOSI: %2d   MISO: %2d   SCLK: %2d   CS: %2d",
             bus_config.mosi_io_num, bus_config.miso_io_num,
             bus_config.sclk_io_num, device_config.cs_io_num);

    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &bus_config, 1));

    // Add device to the SPI bus
    esp_flash_t *ext_flash;
    ESP_ERROR_CHECK(spi_bus_add_flash_device(&ext_flash, &device_config));

    // Probe the Flash chip and initialize it
    esp_err_t err = esp_flash_init(ext_flash);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize external Flash: %s (0x%x)", esp_err_to_name(err), err);
        return NULL;
    }

    // Print out the ID and size
    uint32_t id;
    ESP_LOGI(TAG, "test size = %d KB", ext_flash->size);
    ESP_ERROR_CHECK(esp_flash_read_id(ext_flash, &id));
    ESP_LOGI(TAG, "Initialized external Flash, size=%d KB, ID=0x%x", ext_flash->size / 1024, id);

    return ext_flash;
}

static const esp_partition_t *add_partition(esp_flash_t *ext_flash, const char *partition_label)
{
    ESP_LOGI(TAG, "Adding external Flash as a partition, label=\"%s\", size=%d KB", partition_label, ext_flash->size / 1024);
    const esp_partition_t *fat_partition;
    ESP_ERROR_CHECK(esp_partition_register_external(ext_flash, 0, ext_flash->size, partition_label, ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, &fat_partition));
    return fat_partition;
}

static void list_data_partitions(void)
{
    ESP_LOGI(TAG, "Listing data partitions:");
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL; it = esp_partition_next(it))
    {
        const esp_partition_t *part = esp_partition_get(it);
        ESP_LOGI(TAG, "- partition '%s', subtype %d, offset 0x%x, size %d kB",
                 part->label, part->subtype, part->address, part->size / 1024);
    }

    esp_partition_iterator_release(it);
}

static bool mount_fatfs(const char *partition_label)
{
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    const esp_vfs_fat_mount_config_t mount_config = {
        .max_files = 4,
        .format_if_mount_failed = true,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE};
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, partition_label, &mount_config, &s_wl_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return false;
    }
    return true;
}

static void get_fatfs_usage(size_t *out_total_bytes, size_t *out_free_bytes)
{
    FATFS *fs;
    size_t free_clusters;
    int res = f_getfree("0:", &free_clusters, &fs);
    assert(res == FR_OK);
    size_t total_sectors = (fs->n_fatent - 2) * fs->csize;
    size_t free_sectors = free_clusters * fs->csize;

    // assuming the total size is < 4GiB, should be true for SPI Flash
    if (out_total_bytes != NULL)
    {
        *out_total_bytes = total_sectors * fs->ssize;
    }
    if (out_free_bytes != NULL)
    {
        *out_free_bytes = free_sectors * fs->ssize;
    }
}

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
