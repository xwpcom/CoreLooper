#!/bin/sh

runCMD() { # usage: runCMD "<command>" "<retVal>"
    TMP_FILE=$(mktemp)
    eval $1 > $TMP_FILE 2>&1
    RETVAL=$?
    if [ "$RETVAL" != "$2" ]; then
        echo "Command ($1) returned ${RETVAL}, but expected $2. Error output:"
        cat $TMP_FILE
        exit 1
    fi
}

# Successful tests
runCMD "ldd /lib/libustream-ssl.so" 0
# Temporary workaround: comment out missing kmods repo line for 21.02 specifically.
# Remove after fixed upstream.
runCMD "sed '\/src\/gz openwrt_kmods https:\/\/downloads.openwrt.org\/releases\/21.02-SNAPSHOT\/targets\/x86\/64\/kmods\/5.4.238-1-5a722da41bc36de95a7195be6fce1b45/s//#&/' -i /etc/opkg/distfeeds.conf" 0
runCMD "opkg update" 0
runCMD "uclient-fetch 'https://letsencrypt.org'" 0
# Negative tests
runCMD "uclient-fetch --ca-certificate=/dev/null 'https://letsencrypt.org'" 5
runCMD "uclient-fetch 'https://self-signed.badssl.com/'" 5
runCMD "uclient-fetch 'https://untrusted-root.badssl.com/'" 5
runCMD "uclient-fetch 'https://expired.badssl.com/'" 5

echo "All tests passed."
