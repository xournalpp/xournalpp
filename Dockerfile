FROM ubuntu:22.04
RUN apt-get update
RUN apt-get install -y build-essential cmake libgtk-3-dev libpoppler-glib-dev portaudio19-dev libsndfile-dev \
  dvipng texlive libxml2-dev liblua5.3-dev libzip-dev librsvg2-dev gettext lua-lgi \
  libgtksourceview-4-dev
RUN mkdir /home/xournalpp/
WORKDIR /home/xournalpp/
CMD ["./home/xournalpp/scripts/build.sh"]
