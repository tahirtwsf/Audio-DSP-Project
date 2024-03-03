fs = 48000; % sample rate
f1 = 250;
N = 700; % number of filter coefficients
win = hamming(N)'; % window: transposed so .* can be used
wc = (f1 / (fs / 2)) * pi; % cutoff frequency
n = 0:N-1;
L = (N-1)/2;
hLP = win .* ((wc/pi) * sinc((wc/pi) * (n - L)));
figure; stem(n, hLP);
figure; freqz(hLP, 1);
subplot(2,1,1); refline(0,-3); % Add a -3 dB reference line
figure; zplane(hLP, 1);
