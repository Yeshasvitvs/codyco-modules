#!/usr/bin/lua

require("rfsm")
require("yarp")

yarp.Network()

--action_status = 'idle'

----events
event_no_contact = "e_no_contacts_on_hands"
event_contact_on_left_hand = "e_contacts_only_on_left_hand"
event_contact_on_right_hand = "e_contacts_only_on_right_hand"
event_contacts_on_both_hands = "e_contacts_on_both_hands"

----definitions
----copied from skinDynLib common.h
bodyPart_left_arm  = 3
bodyPart_right_arm = 4

-------
function update_skin_events()
   while true do
        --Use last received skinContactsList
        skin_contacts = event_port:read(false)
        if skin_contacts ~= nil then
            buffer_skin_contacts = skin_contacts
        end

        --Count contacts on left_arm and right_arm
        contact_left_arm = 0
        contact_right_arm = 0
        for i = 1,buffer_skin_contacts:size() do
            bp_contact = buffer_skin_contacts:get(i):asList():get(0):asList():get(1)
            if( bp_contact == bodyPart_left_arm ) then
                contact_left_arm = contact_left_arm + 1
            end
            if( bp_contact == bodyPart_right_arm ) then
                contact_right_arm = contact_right_arm + 1
            end
        end

        -- Produce proper events given the contact state
        if( contact_left_arm > 0 and contact_right_arm > 0 ) then
            event_to_send = event_contacts_on_both_hands
        end
        if( contact_left_arm == 0 and contact_right_arm > 0 ) then
            event_to_send = event_contact_on_right_hand
        end
        if( contact_left_arm > 0 and contact_right_arm == 0 ) then
            event_to_send = event_contact_on_left_hand
        end
        if( contact_left_arm == 0 and contact_right_arm == 0 ) then
            event_to_send = event_no_contact
        end

        rfsm.send_events(fsm, event_to_send)
        if coroutine.yield() == true then break end
   end
end



-------
shouldExit = false

-- initilization
cmd_action_rpc = yarp.RpcClient()
cmd_action_rpc:open("/codycoCoordinator1Y/cmd_action:o")

event_port = yarp.BufferedPortBottle()
event_port:open("/codycoCoordinator1Y/skin_events:i")

-- load state machine model and initalize it
fsm_model = rfsm.load("./fsm_codycoCoordinator1Y.lua")
fsm = rfsm.init(fsm_model)

co_updater = coroutine.create(update_events)

repeat
    coroutine.resume(co_updater)
    rfsm.run(fsm)
    yarp.Time_delay(0.1)
until shouldExit ~= false

print("finishing")
coroutine.resume(co_updater, true)

event_port:close()
cmd_action_rpc:close()

-- Deinitialize yarp network
yarp.Network_fini()
