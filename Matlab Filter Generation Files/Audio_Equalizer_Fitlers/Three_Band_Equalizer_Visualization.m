figure
fs = 48000;
[H1, W] = freqz(hLP, 1);
[H2, W] = freqz(hBP, 1);
[H3, W] = freqz(hHP, 1);
plot(W/pi * fs/2, abs(H1),'k'); hold on
plot(W/pi * fs/2, abs(H2),'r');
plot(W/pi * fs/2, abs(H3),'b');
xlim([0, fs/2]);