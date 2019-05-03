// Socks4a Client library
//
// Connects to Socks servers such as the Tor Socks port
//
// Specs: http://socks-relay.sourceforge.net/socks4.protocol.txt
//        http://socks-relay.sourceforge.net/socks4a.protocol.txt
//
// In violation of the specs, we don't send the username as identd servers have died out
// And we don't expect the Tor client to bother with the local username.
//
// Inspired by: Adam Langley's https://code.google.com/r/matatayaya-study/source/browse/proxy/socks5.go
//
// Copyright 2015, Guido Witmond <guido@witmond.nl>
// Licensed under MIT license. See LICENSE

package main

import (
	"errors"
	"io"
	"log"
	"net"
	"strconv"
  "os"
)

const socks4aVersion = 4
const socks4aConnect = 1
const socks4aGranted = 90
const socks4aRejected = 91
const socks4aMissingIdentd = 92
const socks4aFailedIdentd = 93

type Socks4a struct {
	Network string  // "tcp"
	Address string  // "127.0.0.1:9050" or the address of your Socks4a proxy (the tor SocksPort)
}

func (s *Socks4a) Dial (destination string) (net.Conn, error) {
	destStr, portStr, err := net.SplitHostPort(destination)
	if err != nil {
		return nil, err
	}
	dest := []byte(destStr)

	port, err := strconv.Atoi(portStr)
	if err != nil {
		return nil, errors.New("proxy: failed to parse port number: " + portStr)
	}
	if port < 1 || port > 0xffff {
		return nil, errors.New("proxy: port number out of range: " + portStr)
	}

	// create the CONNECT message
	buf := make([]byte, 11 + len(dest))
	buf[0] = socks4aVersion
	buf[1] = socks4aConnect
	buf[2] = byte(port >> 8) // Network byte order is Big endian
	buf[3] = byte(port)
	buf[4] = 0 // 4-7: ip-address 0.0.0.1
	buf[5] = 0
	buf[6] = 0
	buf[7] = 1
	buf[8] = 65 // char('A') for username, assuming Tor doesn't validate usernames
	buf[9] = 0
	// copy the address
	for i, c := range dest {
		buf[10+i] = c
	}
	buf[10+len(dest)] = 0 // terminate with a \0

	// connect to Tor
	conn, err := net.Dial(s.Network, s.Address)
		if err != nil {
			return nil, err
		}

	// Close connection at errors.
	closeConn := &conn
	defer func() {
		if closeConn != nil {
			(*closeConn).Close()
		}
	}()

	// Send the CONNECT message
	_, err = conn.Write(buf);
	if err != nil {
		return nil, errors.New("proxy: failed to write CONNECT message to SOCKS4a proxy at " + s.Address + ": " + err.Error())
	}

	// Read up to 8 bytes
	n, err := io.ReadFull(conn, buf[:8])
	log.Printf("Read %d bytes from tor socks port.", n)
	if err != nil {
		return nil, errors.New("proxy: failed to connect SOCKS4a proxy at " + s.Address + ": " + err.Error())
	}

	// Tor doesn't return the socks version at byte 0, just a 0
	//if buf[0] != socks4aVersion and buf[0] != 0 {
	if buf[0] != 0 {
		return nil, errors.New("proxy: SOCKS4a proxy at " + s.Address + " has unexpected version " + strconv.Itoa(int(buf[0])))
	}
	if buf[1] != socks4aGranted {
		return nil, errors.New("proxy: SOCKS4a proxy at " + s.Address + " failed acceptance")
	}

	// Ignore the port and address 'fields' in the message, they're empty anyway.

	// we have our connection granted, speak freely.
	closeConn = nil // keep it open
	return conn, nil
}

func main() {
  torsocks := "127.0.0.1:12321"
	destination := "httpbin.org:80"

	s := &Socks4a {
		Network: "tcp",
		Address: torsocks,
	}

	conn, err := s.Dial(destination)
	if err != nil {
		panic(err)
	}
	conn.Write([]byte("GET / HTTP/1.0\r\nHost: baidu.com\r\n\r\n"))
	n, err := io.Copy(os.Stdout, conn)
	if err != nil {
		panic(err)
	}
	log.Printf("copying %v bytes with error %v", n, err)
	conn.Close()
}
