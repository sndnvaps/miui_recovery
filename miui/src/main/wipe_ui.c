#include "../miui_inter.h"
#include "../miui.h"
#include "../../../miui_intent.h"

#define WIPE_FACTORY     1
#define WIPE_DATA        2
#define WIPE_CACHE       3
#define WIPE_DALVIK      4
#define FORMAT_SYSTEM    11
#define FORMAT_DATA      12
#define FORMAT_CACHE     13
#define FORMAT_BOOT      14
#define FORMAT_SDCARD    15
#define FORMAT_ALL       16
#ifdef DUALSYSTEM_PARTITIONS
#define FORMAT_SYSTEM1   17
#define FORMAT_BOOT1     18
 extern int is_tdb_enabled();
#endif

STATUS wipe_item_show(menuUnit *p)
{
#ifdef DUALSYSTEM_PARTITIONS
     int wipe_system_num;
     if (is_tdb_enabled()) {
         if (p->result == WIPE_FACTORY) {
             if (RET_YES == miui_confirm(5, "<~choose.system.title>", "<~choose.system.text>", "@alert", "<~choice.system0.name>", "<~choice.system1.name>")) {
                 wipe_system_num = 0;
             } else {
                 wipe_system_num = 1;
             }
         }
     }
 #endif
	
    if (RET_YES == miui_confirm(3, p->name, p->desc, p->icon)) {
        miui_busy_process();
        switch(p->result) {
            case WIPE_FACTORY:
#ifdef DUALSYSTEM_PARTITIONS
                 if (is_tdb_enabled()) {
                     miuiIntent_send(INTENT_MOUNT, 1, "/data");
                     if (wipe_system_num == 0) {
                         __system("rm -rf /data/system0");
                     } else {
                         __system("rm -rf /data/system1");
                     }
                 } else {
                     miuiIntent_send(INTENT_WIPE, 1, "/cache");
                     miuiIntent_send(INTENT_WIPE, 1, "/data");
                }
 #else
		    
                miuiIntent_send(INTENT_WIPE, 1, "/cache");
                miuiIntent_send(INTENT_WIPE, 1, "/data");
                
#endif
		break;
		
            case WIPE_DATA:
                miuiIntent_send(INTENT_WIPE, 1, "/data");
                break;
            case WIPE_CACHE:
                miuiIntent_send(INTENT_WIPE, 1, "/cache");
                break;
            case WIPE_DALVIK:
                miuiIntent_send(INTENT_WIPE, 1, "dalvik-cache");
                break;
            case FORMAT_SYSTEM:
                miuiIntent_send(INTENT_FORMAT, 1, "/system");
                break;
#ifdef DUALSYSTEM_PARTITIONS
            case FORMAT_SYSTEM1:
                 miuiIntent_send(INTENT_FORMAT, 1, "/system1");
                 break;
 #endif
		
            case FORMAT_DATA:
                miuiIntent_send(INTENT_FORMAT, 1, "/data");
                break;
            case FORMAT_CACHE:
                miuiIntent_send(INTENT_FORMAT, 1, "/cache");
                break;
            case FORMAT_BOOT:
                miuiIntent_send(INTENT_FORMAT, 1, "/boot");
                break;
#ifdef DUALSYSTEM_PARTITIONS
            case FORMAT_BOOT1:
                 miuiIntent_send(INTENT_FORMAT, 1, "/boot1");
                 break;
#endif
		
            case FORMAT_SDCARD:
                miuiIntent_send(INTENT_FORMAT, 1, "/sdcard");
                break;
            case FORMAT_ALL:
                miuiIntent_send(INTENT_FORMAT, 1, "/system");
                miuiIntent_send(INTENT_FORMAT, 1, "/data");
                miuiIntent_send(INTENT_FORMAT, 1, "/cache");
                break;
            default:
                assert_if_fail(0);
                break;
        }
    }

    return MENU_BACK;

}

STATUS wipe_menu_show(menuUnit *p)
{
     return_val_if_fail(p != NULL, RET_FAIL);
     int n = p->get_child_count(p);
     int selindex = 0;
     return_val_if_fail(n >= 1, RET_FAIL);
     return_val_if_fail(n < ITEM_COUNT, RET_FAIL);
     struct _menuUnit* temp = p->child;
     return_val_if_fail(temp != NULL, RET_FAIL);
     char **menu_item = malloc(n * sizeof(char *));
     assert_if_fail(menu_item != NULL);
     int i = 0;
     for (i = 0; i < n; i++)
     {
         menu_item[i] = temp->name;
         temp = temp->nextSilbing;
     }
     selindex = miui_mainmenu(p->name, menu_item, NULL, NULL, n);
     p->result = selindex;
     if (menu_item != NULL) free(menu_item);
     return p->result;
}
 struct _menuUnit* wipe_ui_init()
 {
     struct _menuUnit* p = common_ui_init();
     return_null_if_fail(p != NULL);
     return_null_if_fail(menuUnit_set_name(p, "<~wipe.name>") == RET_OK);
     return_null_if_fail(menuUnit_set_title(p, "<~wipe.title>") == RET_OK);
     return_null_if_fail(menuUnit_set_icon(p, "@wipe") == RET_OK);
     return_null_if_fail(RET_OK == menuUnit_set_show(p, &wipe_menu_show));
     return_null_if_fail(menuNode_init(p) != NULL);
     //factory reset
     struct _menuUnit* temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~wipe.factory.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, WIPE_FACTORY) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    
    
    //wipe_data
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~wipe.data.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, WIPE_DATA) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //wipe_cache
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~wipe.cache.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, WIPE_CACHE) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //wipe dalvik-cache
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~wipe.dalvik-cache.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, WIPE_DALVIK) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format system
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.system.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_SYSTEM) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
#ifdef DUALSYSTEM_PARTITIONS
     //format system1
     temp = common_ui_init();
     assert_if_fail(menuNode_add(p, temp) == RET_OK);
     return_null_if_fail(menuUnit_set_name(temp, "<~format.system1.name>") == RET_OK);
     return_null_if_fail(menuUnit_set_result(temp, FORMAT_SYSTEM1) == RET_OK);
     return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
#endif
      
    //format data
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.data.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_DATA) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format cache
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.cache.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_CACHE) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format BOOT
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.boot.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_BOOT) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
#ifdef DUALSYSTEM_PARTITIONS
     //format BOOT1
     temp = common_ui_init();
     assert_if_fail(menuNode_add(p, temp) == RET_OK);
     return_null_if_fail(menuUnit_set_name(temp, "<~format.boot1.name>") == RET_OK);
     return_null_if_fail(menuUnit_set_result(temp, FORMAT_BOOT) == RET_OK);
     return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
#endif
   /* for debug 
    //format SDCARD
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.sdcard.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_SDCARD) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
    //format ALL
    temp = common_ui_init();
    assert_if_fail(menuNode_add(p, temp) == RET_OK);
    return_null_if_fail(menuUnit_set_name(temp, "<~format.all.name>") == RET_OK);
    return_null_if_fail(menuUnit_set_result(temp, FORMAT_ALL) == RET_OK);
    return_null_if_fail(RET_OK == menuUnit_set_show(temp, &wipe_item_show));
 */
    return p;
 }




