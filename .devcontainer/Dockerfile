# Use an official Ubuntu as a parent image
FROM ubuntu:22.04

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y zsh build-essential curl git netcat libx11-dev libgtkmm-3.0-dev pkg-config g++ && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install Oh My Zsh
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

# Set Zsh as the default shell
RUN chsh -s $(which zsh)

CMD ["zsh"]
