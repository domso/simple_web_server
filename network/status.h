#pragma once

namespace network {    
    enum status {
        ok,
        retry_read,
        retry_write,
        error
    };
}
