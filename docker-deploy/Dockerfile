From gcc
# Install Boost
# https://www.boost.org/doc/libs/1_80_0/more/getting_started/unix-variants.html
RUN echo "yes" && \
    cd /tmp && \
    BOOST_VERSION_MOD=$(echo $BOOST_VERSION | tr . _) && \
    wget https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    tar --bzip2 -xf boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    cd boost_${BOOST_VERSION_MOD} && \
    ./bootstrap.sh --prefix=/usr/local && \
    ./b2 install && \
    echo "hello" &&\
    rm -rf /tmp/*
RUN mkdir /var/log/erss
RUN mkdir /HTTPProxy
RUN echo "echo"
WORKDIR ./HTTPProxy
ADD ./HTTPProxy /HTTPProxy
