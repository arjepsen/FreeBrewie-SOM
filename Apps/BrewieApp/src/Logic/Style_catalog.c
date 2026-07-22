#include "Style_catalog.h"

#include <stdio.h>
#include <string.h>

#define STYLE_CATALOG_JSON_MAX_BYTES 4096U
#define STYLE_CATALOG_PATH_COUNT 4U

typedef struct
{
    /** Draft-owned copies point at these stable bounded buffers. */
    char style_name[STYLE_CATALOG_TEXT_MAX_LENGTH];
    char style_number[STYLE_CATALOG_TEXT_MAX_LENGTH];
    char style_category[STYLE_CATALOG_TEXT_MAX_LENGTH];
    char style_type[STYLE_CATALOG_TEXT_MAX_LENGTH];
} style_catalog_storage_t;

static bool style_catalog_try_load_file(const char *path);
static void style_catalog_load_fallback();
static bool style_catalog_parse_json(const char *json_text);
static const char *style_catalog_find_object_end(const char *object_start);
static bool style_catalog_copy_json_string(const char *object_start,
                                           const char *object_end,
                                           const char *field_name,
                                           char *destination,
                                           size_t destination_size);
static const char *style_catalog_find_field_value(const char *object_start,
                                                  const char *object_end,
                                                  const char *field_name);
static void style_catalog_store_option(uint8_t index,
                                       const char *style_name,
                                       const char *style_number,
                                       const char *style_category,
                                       const char *style_type);

static const char *const style_catalog_paths[STYLE_CATALOG_PATH_COUNT] = {
    "/opt/brewie/Data/styles.json",
    "Data/styles.json",
    "../Data/styles.json",
    "../../Data/styles.json"};

static const style_catalog_style_t style_catalog_fallback_styles[] = {
    {"Belgian IPA", "21B", "IPA", "Ale"},
    {"American Pale Ale", "18B", "Pale American Ale", "Ale"},
    {"Bohemian Pilsner", "3B", "Pale Lager", "Lager"},
    {"Irish Stout", "15B", "Dark British Beer", "Ale"},
    {"Hefeweizen", "10A", "German Wheat Beer", "Ale"},
    {"Witbier", "24A", "Belgian Ale", "Ale"}};

static style_catalog_storage_t style_catalog_storage[STYLE_CATALOG_MAX_STYLES];
static style_catalog_style_t style_catalog_styles[STYLE_CATALOG_MAX_STYLES];
static uint8_t style_catalog_count;
static bool style_catalog_initialized;
static bool style_catalog_file_loaded;

/****************************************************************************************
 * @brief Initialize the bounded style catalog cache.
 *
 * The first implementation reads a small project-owned JSON file. If that file is not
 * present on the SOM yet, fallback starter records keep the UI usable during bring-up.
 ****************************************************************************************/
bool style_catalog_init()
{
    uint8_t path_index;

    if (style_catalog_initialized)
    {
        return (style_catalog_count > 0U);
    }

    style_catalog_initialized = true;
    for (path_index = 0U; path_index < STYLE_CATALOG_PATH_COUNT; ++path_index)
    {
        if (style_catalog_try_load_file(style_catalog_paths[path_index]))
        {
            style_catalog_file_loaded = true;
            return true;
        }
    }

    style_catalog_load_fallback();
    style_catalog_file_loaded = false;
    return (style_catalog_count > 0U);
}

/****************************************************************************************
 * @brief Return the number of style records currently available.
 ****************************************************************************************/
uint8_t style_catalog_get_count()
{
    if (!style_catalog_initialized)
    {
        style_catalog_init();
    }

    return style_catalog_count;
}

/****************************************************************************************
 * @brief Return one style record by index, or NULL when out of range.
 ****************************************************************************************/
const style_catalog_style_t *style_catalog_get_style(uint8_t index)
{
    if (!style_catalog_initialized)
    {
        style_catalog_init();
    }

    if (index >= style_catalog_count)
    {
        return NULL;
    }

    return &style_catalog_styles[index];
}

/****************************************************************************************
 * @brief Report whether the current catalog came from styles.json.
 ****************************************************************************************/
bool style_catalog_loaded_from_file()
{
    if (!style_catalog_initialized)
    {
        style_catalog_init();
    }

    return style_catalog_file_loaded;
}

/****************************************************************************************
 * @brief Try to read and parse one candidate styles.json path.
 ****************************************************************************************/
static bool style_catalog_try_load_file(const char *path)
{
    static char json_buffer[STYLE_CATALOG_JSON_MAX_BYTES + 1U];
    FILE *file;
    size_t bytes_read;

    file = fopen(path, "rb");
    if (file == NULL)
    {
        return false;
    }

    bytes_read = fread(json_buffer, 1U, STYLE_CATALOG_JSON_MAX_BYTES, file);
    fclose(file);
    if (bytes_read == 0U || bytes_read >= STYLE_CATALOG_JSON_MAX_BYTES)
    {
        return false;
    }

    json_buffer[bytes_read] = '\0';
    return style_catalog_parse_json(json_buffer);
}

/****************************************************************************************
 * @brief Fill the catalog from tiny built-in starter data.
 ****************************************************************************************/
static void style_catalog_load_fallback()
{
    uint8_t index;
    uint8_t fallback_count;

    fallback_count = (uint8_t)(sizeof(style_catalog_fallback_styles) / sizeof(style_catalog_fallback_styles[0]));
    style_catalog_count = 0U;
    for (index = 0U; index < fallback_count && index < STYLE_CATALOG_MAX_STYLES; ++index)
    {
        style_catalog_store_option(index,
                                   style_catalog_fallback_styles[index].style_name,
                                   style_catalog_fallback_styles[index].style_number,
                                   style_catalog_fallback_styles[index].style_category,
                                   style_catalog_fallback_styles[index].style_type);
        style_catalog_count = (uint8_t)(style_catalog_count + 1U);
    }
}

/****************************************************************************************
 * @brief Parse the project-owned styles.json file into bounded style records.
 *
 * This is intentionally not a general JSON parser. It accepts the small object array we
 * own in Data/styles.json and ignores anything beyond STYLE_CATALOG_MAX_STYLES.
 ****************************************************************************************/
static bool style_catalog_parse_json(const char *json_text)
{
    const char *cursor;
    const char *object_end;
    uint8_t index;

    if (json_text == NULL)
    {
        return false;
    }

    index = 0U;
    cursor = json_text;
    while (index < STYLE_CATALOG_MAX_STYLES)
    {
        cursor = strchr(cursor, '{');
        if (cursor == NULL)
        {
            break;
        }

        object_end = style_catalog_find_object_end(cursor);
        if (object_end == NULL)
        {
            break;
        }

        if (style_catalog_copy_json_string(cursor,
                                           object_end,
                                           "name",
                                           style_catalog_storage[index].style_name,
                                           sizeof(style_catalog_storage[index].style_name)) &&
            style_catalog_copy_json_string(cursor,
                                           object_end,
                                           "number",
                                           style_catalog_storage[index].style_number,
                                           sizeof(style_catalog_storage[index].style_number)) &&
            style_catalog_copy_json_string(cursor,
                                           object_end,
                                           "category",
                                           style_catalog_storage[index].style_category,
                                           sizeof(style_catalog_storage[index].style_category)) &&
            style_catalog_copy_json_string(cursor,
                                           object_end,
                                           "type",
                                           style_catalog_storage[index].style_type,
                                           sizeof(style_catalog_storage[index].style_type)))
        {
            style_catalog_styles[index].style_name = style_catalog_storage[index].style_name;
            style_catalog_styles[index].style_number = style_catalog_storage[index].style_number;
            style_catalog_styles[index].style_category = style_catalog_storage[index].style_category;
            style_catalog_styles[index].style_type = style_catalog_storage[index].style_type;
            index = (uint8_t)(index + 1U);
        }

        cursor = object_end + 1;
    }

    style_catalog_count = index;
    return (style_catalog_count > 0U);
}

/****************************************************************************************
 * @brief Find the closing brace for one flat style object.
 ****************************************************************************************/
static const char *style_catalog_find_object_end(const char *object_start)
{
    return strchr(object_start, '}');
}

/****************************************************************************************
 * @brief Copy one JSON string field from a flat object.
 ****************************************************************************************/
static bool style_catalog_copy_json_string(const char *object_start,
                                           const char *object_end,
                                           const char *field_name,
                                           char *destination,
                                           size_t destination_size)
{
    const char *value_start;
    const char *value_end;
    size_t value_length;

    if (destination == NULL || destination_size == 0U)
    {
        return false;
    }

    value_start = style_catalog_find_field_value(object_start, object_end, field_name);
    if (value_start == NULL)
    {
        destination[0] = '\0';
        return false;
    }

    value_end = value_start;
    while (value_end < object_end && *value_end != '"')
    {
        ++value_end;
    }

    value_length = (size_t)(value_end - value_start);
    if (value_length >= destination_size)
    {
        value_length = destination_size - 1U;
    }

    memcpy(destination, value_start, value_length);
    destination[value_length] = '\0';
    return true;
}

/****************************************************************************************
 * @brief Find the first character inside a named string value.
 ****************************************************************************************/
static const char *style_catalog_find_field_value(const char *object_start,
                                                  const char *object_end,
                                                  const char *field_name)
{
    char key_text[32];
    const char *cursor;

    snprintf(key_text, sizeof(key_text), "\"%s\"", field_name);
    cursor = strstr(object_start, key_text);
    if (cursor == NULL || cursor >= object_end)
    {
        return NULL;
    }

    cursor += strlen(key_text);
    while (cursor < object_end && *cursor != ':')
    {
        ++cursor;
    }

    if (cursor >= object_end)
    {
        return NULL;
    }

    ++cursor;
    while (cursor < object_end && (*cursor == ' ' || *cursor == '\n' || *cursor == '\r' || *cursor == '\t'))
    {
        ++cursor;
    }

    if (cursor >= object_end || *cursor != '"')
    {
        return NULL;
    }

    return cursor + 1;
}

/****************************************************************************************
 * @brief Copy fallback strings into the same bounded storage used by file-loaded styles.
 ****************************************************************************************/
static void style_catalog_store_option(uint8_t index,
                                       const char *style_name,
                                       const char *style_number,
                                       const char *style_category,
                                       const char *style_type)
{
    snprintf(style_catalog_storage[index].style_name,
             sizeof(style_catalog_storage[index].style_name),
             "%s",
             style_name);
    snprintf(style_catalog_storage[index].style_number,
             sizeof(style_catalog_storage[index].style_number),
             "%s",
             style_number);
    snprintf(style_catalog_storage[index].style_category,
             sizeof(style_catalog_storage[index].style_category),
             "%s",
             style_category);
    snprintf(style_catalog_storage[index].style_type,
             sizeof(style_catalog_storage[index].style_type),
             "%s",
             style_type);
    style_catalog_styles[index].style_name = style_catalog_storage[index].style_name;
    style_catalog_styles[index].style_number = style_catalog_storage[index].style_number;
    style_catalog_styles[index].style_category = style_catalog_storage[index].style_category;
    style_catalog_styles[index].style_type = style_catalog_storage[index].style_type;
}
