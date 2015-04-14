# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|

  # Every Vagrant development environment requires a base box. This is the box
  # which is provisioned.
  config.vm.box = "jon-whit/herbert-os"

  # Configure Windows remote management.
  config.vm.communicator = "winrm"
  config.winrm.username = "vagrant"
  config.winrm.password = "vagrant"

  # Network specific configurations.
  config.vm.network :forwarded_port, guest: 3389, host: 3389, id: "rdp", auto_correct: true

  # Provider-specific configuration
   config.vm.provider "virtualbox" do |vb|

     # Customize the name of the of VM in VirtualBox:
     vb.name = "herbert-os"

     # Customize the CPU count and amount of memory on the VM:
     vb.cpus = 2
     vb.memory = "1024"

     # add USB filter to attach PGR Chameleon camera
     vb.customize ['modifyvm', :id, '--usb', 'on']
     vb.customize ['usbfilter', 'add', '0', '--target', :id, '--name', 'Chameleon USB 2.0', '--vendorid', '0x1e10', '--productid', '0x2004']
   end

  # Enable provisioning with a shell script. This will execute in Windows
  # Powershell.
  config.vm.provision "shell", inline: <<-SHELL

   SHELL
end
