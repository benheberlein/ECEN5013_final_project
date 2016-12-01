
ip, nm, gw=wifi.sta.getip()
print("Wireless Configuration")
print("IP address: ",ip)
print("Netmask: ",nm)
print("Gateway Address: ", gw,'\n')

srv=net.createServer(net.TCP) 
srv:listen(8888, ip, function(sck)
    sck:on("receive", function(sck, payload)
        print("Received data payload.")
        print(payload)

        local response = {}

        response[#response + 1] = "Data"
        response[#response + 1] = "More data"
        response[#response + 1] = "Lots of data"

        local function send(sk)
            if #response > 0
                then sk:send(table.remove(response, 1))
            else
                sk.close()
                response = nil
            end
        end

        sck:on("sent", send)
    end)
end)


