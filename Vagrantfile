Vagrant.configure("2") do |config|


  config.vm.hostname = "hft-exchange"

  config.vm.provider "docker" do |d, override|
    d.build_dir = "."
    d.remains_running = true
    d.has_ssh = true
  end

  config.vm.define "ome1" do |ome|
    ome.vm.boot_timeout = 1200
    ome.vm.hostname = "order-matching-engine"

    ome.vm.provision "shell", path: "init-vm.sh"
    ome.vm.synced_folder "./order-matching-engine", "/order-matching-engine", mount_options: ["dmode=777", "fmode=777"]

  end

  config.vm.define "gateway1" do |gateway|
    gateway.vm.boot_timeout = 1200
    gateway.vm.hostname = "gateway"

    gateway.vm.provision "shell", path: "init-vm.sh"
    gateway.vm.synced_folder "./gateway", "/gateway", mount_options: ["dmode=777", "fmode=777"]
  end

  config.vm.define "tp1" do |tp|
    tp.vm.boot_timeout = 1200
    tp.vm.hostname = "tickerplant"

    tp.vm.provision "shell", path: "init-vm.sh"
    tp.vm.synced_folder "./tickerplant", "/tickerplant", mount_options: ["dmode=777", "fmode=777"]
  end


end