#ifndef APEP_I18N_H
#define APEP_I18N_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ----------------------------
    Localization system
    ---------------------------- */

    /**
     * Initialize the localization system.
     * @param locale Language code (e.g., "en", "cs", NULL for auto-detect)
     * @param locales_dir Directory containing .loc files (NULL for default "locales")
     * @return 0 on success, -1 on error
     */
    int apep_i18n_init(const char *locale, const char *locales_dir);

    /**
     * Get localized string for a given key.
     * If key is not found, returns the key itself (fallback behavior).
     * @param key UTF-8 string key
     * @return Localized string (never NULL)
     */
    const char *apep_i18n_get(const char *key);

    /**
     * Shorthand macro for getting localized strings.
     */
#define _(key) apep_i18n_get(key)

    /**
     * Set the current locale.
     * @param locale Language code (e.g., "en", "cs")
     * @return 0 on success, -1 on error
     */
    int apep_i18n_set_locale(const char *locale);

    /**
     * Get the current locale.
     * @return Current locale string (never NULL)
     */
    const char *apep_i18n_get_locale(void);

    /**
     * Clean up localization resources.
     */
    void apep_i18n_cleanup(void);

    /**
     * Detect system locale.
     * @return Detected locale code (e.g., "en", "cs", "fr")
     */
    const char *apep_i18n_detect_system_locale(void);

#ifdef __cplusplus
}
#endif

#endif /* APEP_I18N_H */
