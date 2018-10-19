
#ifndef __DATA_STORAGE_H__
#define __DATA_STORAGE_H__

extern "C"
{
#include "fds.h"
}

#define FILE_ID     0x1111
#define REC_KEY_UP_POS     0x2222
#define REC_KEY_DOWN_POS     0x0002

class DataStorage {
public:
    DataStorage()
    {
       // _write_flag = false;
        ret_code_t ret = fds_register(&handle_fds_events);
        MBED_ASSERT(ret == FDS_SUCCESS);
        ret = fds_init();
        MBED_ASSERT(ret == FDS_SUCCESS);
    }
    
    bool write_up_position(uint32_t pos)
    {
        uint32_t value =    pos;
        fds_record_t        record;
        fds_record_desc_t   record_desc;
        fds_record_chunk_t  record_chunk;
        // Set up data.
        record_chunk.p_data         = &value;
        record_chunk.length_words   = 2;
        // Set up record.
        record.file_id              = FILE_ID;
        record.key                  = REC_KEY_UP_POS;
        record.data.p_chunks        = &record_chunk;
        record.data.num_chunks      = 1;

     //   _write_flag = false;
        ret_code_t ret = fds_record_write(&record_desc, &record);
     //   while (ret == FDS_SUCCESS && !_write_flag);
        
        return ret != FDS_SUCCESS;
    }

    bool write_bottom_position(int pos)
    {
    }
    
    int read_up_position()
    {
    }
    
    int read_bottom_position()
    {
    }
    
private:
//    static bool _write_flag;
    
    static void handle_fds_events(fds_evt_t const * const p_fds_evt)
    {
        switch (p_fds_evt->id)
        {
        case FDS_EVT_INIT:
            MBED_ASSERT(p_fds_evt->result == FDS_SUCCESS);
            break;
        case FDS_EVT_WRITE:
            MBED_ASSERT(p_fds_evt->result == FDS_SUCCESS);
          //  _write_flag = true;
            break;
        default:
            break;
        }
    }
    
};

#endif /* #ifndef __DATA_STORAGE_H__ */
