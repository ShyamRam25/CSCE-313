from scapy.all import *
from scapy.layers.inet import *


def main():
    """Driver function"""
    while True:
        print_menu()
        option = input('Choose a menu option: ')
        if option == '1':
            print("Creating and sending packets ...")
            # TODO
            numpackets = int(input("How many packets? "))
            interval = int(input("How many seconds in between? "))
            send_pkt(numpackets, interval)
        elif option == '2':
            print("Listening to all traffic to 8.8.4.4 for 1 minute ...")
            # TODO
            pac = sniff(timeout = 60)
            for pkt in pac:
                if IP in pkt and pkt[IP].dst == "8.8.4.4":
                    print_pkt(pkt)
        elif option == '3':
            print("Listening continuously to only ping commands to 8.8.4.4 ...")
            # TODO
            #Sniff continuously only the packets from a ping command to the address 8.8.4.4.
            def print_ping(pkt):
                if IP in pkt and pkt[IP].dst == "8.8.4.4":
                    print("Source IP: ", pkt[IP].src)
                    print("Destination IP: ", pkt[IP].dst)
                    print("Protocol number: ", pkt[IP].proto)
                    print("TTL: ", pkt[IP].ttl)
                    print("Length in bytes: ", pkt[IP].len)
                    if Raw in pkt:
                        print("Raw payload: ", pkt[Raw].load)
                    else:
                        print("Raw payload: None")
                    print("--------------------")
            
            sniff(filter="icmp and host 8.8.4.4", prn=print_ping)
        elif option == '4':
            print("Listening continuously to only outgoing telnet commands ...")
            # TODO'

            def print_filter(pkt):
                if IP in pkt and pkt[TCP].dport == 23 and pkt[IP].dst == "8.8.4.4":
                    print("Source IP: ", pkt[IP].src)
                    print("Destination IP: ", pkt[IP].dst)
                    print("Protocol number: ", pkt[IP].proto)
                    print("TTL: ", pkt[IP].ttl)
                    print("Length in bytes: ", pkt[IP].len)
                    if Raw in pkt:
                        print("Raw payload: ", pkt[Raw].load)
                    else:
                        print("Raw payload: None")
                    print("--------------------")
            pac = sniff(filter = "tcp and host 8.8.4.4", prn=print_filter)

        elif option == '5':
            print("End")
            break
        else:
            print(f"\nInvalid entry\n")


def send_pkt(number, interval):
    """
    Send a custom packet with the following fields

    #### Ethernet layer
    - Source MAC address: 00:11:22:33:44:55
    - Destination MAC address: 55:44:33:22:11:00

    #### IP layer
    - Source address: 192.168.10.4
    - Destination address: 8.8.4.4
    - Protocol: TCP
    - TTL: 26

    #### TCP layer
    - Source port: 23
    - Destination port: 80

    #### Raw payload
    - Payload: "RISC-V Education: https://riscvedu.org/"
    """

    # TODO
    ether = Ether(src="00:11:22:33:44:55", dst="55:44:33:22:11:00")
    ip = IP(src="192.168.10.4", dst="8.8.4.4", ttl=26, proto = "tcp")
    tcp = TCP(sport=23, dport=80)
    payload = Raw(load="RISC-V Education: https://riscvedu.org/")
    pkt = ether/ip/tcp/payload
    sendp(pkt, count=number, inter=interval)
    pass


def print_pkt(packet):
    """ 
    Print Packet fields

    - Source IP
    - Destination IP
    - Protocol number
    - TTL
    - Length in bytes
    - Raw payload (if any)
    """

    print("Source IP: ", packet[IP].src)
    print("Destination IP: ", packet[IP].dst)
    print("Protocol number: ", packet[IP].proto)
    print("TTL: ", packet[IP].ttl)
    print("Length in bytes: ", packet[IP].len)
    if Raw in packet:
        print("Raw payload: ", packet[Raw].load)
    else:
        print("Raw payload: None")
    print("--------------------")
    

    # TODO
    pass


def print_menu():
    """Prints the menu of options"""
    print("*******************Main Menu*******************")
    print('1. Create and send packets')
    print('2. Listen to all traffic to 8.8.4.4 for 1 minute')
    print('3. Listen continuously to only ping commands to 8.8.4.4')
    print('4. Listen continuously to only outgoing telnet commands')
    print('5. Quit')
    print('***********************************************\n')


main()
