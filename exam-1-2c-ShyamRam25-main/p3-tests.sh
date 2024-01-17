#!/usr/bin/env bash

# function to clean up files and make executables
remake () {
    #echo -e "\nCleaning old files and making executables"
    make -s clean
    make -s >/dev/null 2>&1
}

# function to check for IPC files
checkclean () {
    if [ "$1" = "f" ]; then
        if [ "$(find . -type p)" ]; then
            find . -type p -exec rm {} +
        fi
    elif [ "$1" = "l" ]; then
        sleep 2
        if [ "$(find . -type p)" ]; then
            echo -e "\nFailed to close pipes"
            echo -e "  ${RED}Failed${NC}"
        else
            echo -e "  ${GREEN}Test Five Passed${NC}"
            SCORE=$(bc -l <<< "$SCORE+5")
        fi
    else
        echo "something broke"
        exit 1
    fi
}


echo -e "To remove colour from tests, set COLOUR to 1 in sh file\n"
COLOUR=0
if [[ COLOUR -eq 0 ]]; then
    ORANGE='\033[0;33m'
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    NC='\033[0m'
else
    ORANGE='\033[0m'
    GREEN='\033[0m'
    RED='\033[0m'
    NC='\033[0m'
fi

SCORE=0
echo "${SCORE}" > ../file.txt

remake
if [[ $? -eq 0 ]]; then
    echo -e "\nTesting:: Compilation\n"
    echo -e "  ${GREEN}Test One Passed${NC}"
    SCORE=$(bc -l <<< "$SCORE+5")
else
    echo -e "  ${RED}Failed${NC}"
fi

echo -e "\nTesting:: ./mydiff \"ls\" \"ls\"\n"
if [ ! -f "mydiff" ] || [[ $(./mydiff "ls" "ls" 2>&1) ]]; then
    echo -e "  ${RED}Failed${NC}"
else
    echo -e "  ${GREEN}Test Two Passed${NC}"
    SCORE=$(bc -l <<< "$SCORE+10")
fi


remake
echo -e "\nTesting ./mydiff \"cat tests/text-1.txt\" \"cat tests/text-2.txt\"\n"
if diff <(./mydiff "cat tests/text-1.txt" "cat tests/text-2.txt" 2>&1) tests/diff1.txt; then
    echo -e "  ${GREEN}Test Three Passed${NC}"
    SCORE=$(bc -l <<< "$SCORE+15")
else
    echo -e "  ${RED}Failed${NC}"
fi
checkclean "f"

remake
echo -e "\nTesting :: ./mydiff \"grep clean makefile\" \"grep all makefile\"\n"
if diff <(./mydiff "grep clean makefile" "grep all makefile" 2>&1) tests/diff2.txt; then
    echo -e "  ${GREEN}Test Four Passed${NC}"
    SCORE=$(bc -l <<< "$SCORE+15")
else
    echo -e "  ${RED}Failed${NC}"
fi
checkclean "l"


echo -e "\nSCORE: ${SCORE}/50\n"
echo "${SCORE}" > ../file.txt
make -s clean
echo -e "\n"
exit 0