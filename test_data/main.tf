
provider "aws" {
  region     = "us-east-1"
  access_key = ""
  secret_key = ""
}

# 1. Create VPC
resource "aws_vpc" "prod-vpc" {
  cidr_block           = "10.0.0.0/16"
  enable_dns_hostnames = true
  enable_dns_support   = true
  tags = {
    Name = "production"
  }
}

# 2. Create Internet Gateway
resource "aws_internet_gateway" "prod-gw" {
  vpc_id = aws_vpc.prod-vpc.id
}

# 3. Create Custom Route Table
resource "aws_route_table" "prod-rt" {
  vpc_id = aws_vpc.prod-vpc.id

  route {
    # send this traffic...
    # cidr_block = "10.0.1.0/24"
    cidr_block = "0.0.0.0/0"
    # to here:
    gateway_id = aws_internet_gateway.prod-gw.id
  }

  route {
    ipv6_cidr_block = "::/0"
    gateway_id      = aws_internet_gateway.prod-gw.id
  }

  tags = {
    Name = "production"
  }
}

# 4. Create Subnet
resource "aws_subnet" "subnet-1-public" {
  vpc_id            = aws_vpc.prod-vpc.id
  cidr_block        = "10.0.1.0/24"
  availability_zone = "us-east-1a"
  tags = {
    Name = "prod-subnet"
  }
}

# 5. Associate subnet with route Table
resource "aws_route_table_association" "prod-rta" {
  subnet_id      = aws_subnet.subnet-1-public.id
  route_table_id = aws_route_table.prod-rt.id
}

# 6. Create a security group to allow port 22,80,443
resource "aws_security_group" "allow_web" {
  name        = "allow_web_traffic"
  description = "Allow web traffic"
  vpc_id      = aws_vpc.prod-vpc.id

  ingress {
    # from port - to port. If the same, one port
    description = "HTTPS traffic"
    from_port   = 443
    to_port     = 443
    protocol    = "tcp"
    # If you wanted to lock down to just some computers
    # cidr_block = [aws_vpc.prod-vpc.cidr_block] 1.1.1.1/32
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    description = "HTTP traffic"
    from_port   = 80
    to_port     = 80
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  ingress {
    description = "SSH traffic"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  egress {
    from_port = 0
    to_port   = 0
    # -1 means any protocol
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = {
    Name = "Allow_Web"
  }
}

# 7. Create a network interface with an ip in the subnet (from step 4)
resource "aws_network_interface" "web-server-nic" {
  subnet_id = aws_subnet.subnet-1-public.id
  # the IP address within the VPC
  private_ips     = ["10.0.1.50"]
  security_groups = [aws_security_group.allow_web.id]

  # attachment {
  #   instance     = aws_instance.my-first-server.id
  #   device_index = 1
  # }
}

# 8. Assign an elastic IP to the network interface (from step 7)
resource "aws_eip" "ext-ip" {
  vpc                       = true
  network_interface         = aws_network_interface.web-server-nic.id
  associate_with_private_ip = "10.0.1.50"
  depends_on                = [aws_internet_gateway.prod-gw]
  # ^-- this is needed because terraform doesn know if the gateway
  # was created yet and AWS requires that a gateway exists before
  # we can assign the IP. See documentation
}

# 9. Create Ubuntu server and install and enable apache2
resource "aws_instance" "my-first-server" {
  ami               = "ami-0ed9277fb7eb570c9" # data.aws_ami.ubuntu.id
  instance_type     = "t2.micro"
  key_name          = "key2018"
  availability_zone = "us-east-1a"
  network_interface {
    device_index         = 0
    network_interface_id = aws_network_interface.web-server-nic.id
  }

  user_data = <<-EOF
#!/bin/bash
sudo apt update -y
sudo apt install apache2 -y
sudo systemctl start apache2
sudo bash -c 'echo "your very first web server" > /var/www/html/index.html'
  EOF

  # subnet_id         = aws_subnet.subnet-1-public.id
  tags = {
    Name = "WebServer-Terraform"
  }
}
