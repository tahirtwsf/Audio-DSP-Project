fs = 48000; % sample rate
f2 = 2000;
N = 700; % number of filter coefficients
win = hamming(N)'; % window: transposed so .* can be used
wa = (f2 / (fs / 2)) * pi; % acceptance frequency for highpass filter
wc = pi - wa; % translate to lowpass filter cutoff
n = 0:N-1;
L = (N-1)/2;
hLP_tmp = win .* ((wc/pi) * sinc((wc/pi) * (n - L)));
hHP = hLP_tmp .* (-ones(1, length(n))).^n;
figure; stem(n, hHP);
figure; freqz(hHP, 1);
subplot(2,1,1); refline(0,-3); % Add a -3 dB reference line
figure; zplane(hHP, 1);